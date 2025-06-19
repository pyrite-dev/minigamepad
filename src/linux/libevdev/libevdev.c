/*
 * Copyright © 2013 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include <errno.h>
#include <limits.h>
#include <poll.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libevdev-int.h"
#include "libevdev-util.h"
#include "libevdev.h"

enum event_filter_status {
	EVENT_FILTER_NONE,	   /**< Event untouched by filters */
	EVENT_FILTER_MODIFIED, /**< Event was modified */
	EVENT_FILTER_DISCARD,  /**< Discard current event */
};

static int sync_mt_state(struct libevdev* dev, int create_events);

static inline int* slot_value(const struct libevdev* dev, int slot, int axis) {
	if (unlikely(slot > dev->num_slots)) {
		// log_bug(dev, "Slot %d
		// exceeds number of slots (%d)\n", slot, 				dev->num_slots);
		slot = 0;
	}
	if (unlikely(axis < ABS_MT_MIN || axis > ABS_MT_MAX)) {
		log_bug(dev, "MT axis %d is outside the valid range [%d,%d]\n", axis,
				ABS_MT_MIN, ABS_MT_MAX);
		axis = ABS_MT_MIN;
	}
	return &dev->mt_slot_vals[slot * ABS_MT_CNT + axis - ABS_MT_MIN];
}

static int init_event_queue(struct libevdev* dev) {
	const int MIN_QUEUE_SIZE = 256;
	int nevents = 1; /* terminating SYN_REPORT */
	int nslots;
	unsigned int type, code;

	/* count the number of axes, keys, etc. to get a better idea at how
	   many events per EV_SYN we could possibly get. That's the max we
	   may get during SYN_DROPPED too. Use double that, just so we have
	   room for events while syncing a device.
	 */
	for (type = EV_KEY; type < EV_MAX; type++) {
		int max = libevdev_event_type_get_max(type);
		for (code = 0; max > 0 && code < (unsigned int)max; code++) {
			if (libevdev_has_event_code(dev, type, code))
				nevents++;
		}
	}

	nslots = libevdev_get_num_slots(dev);
	if (nslots > 1) {
		int num_mt_axes = 0;

		for (code = ABS_MT_SLOT; code <= ABS_MAX; code++) {
			if (libevdev_has_event_code(dev, EV_ABS, code))
				num_mt_axes++;
		}

		/* We already counted the first slot in the initial count */
		nevents += num_mt_axes * (nslots - 1);
	}

	return queue_alloc(dev, max(MIN_QUEUE_SIZE, nevents * 2));
}

static void libevdev_dflt_log_func(enum libevdev_log_priority priority,
								   void* data, const char* file, int line,
								   const char* func, const char* format,
								   va_list args) {
	const char* prefix;
	switch (priority) {
	case LIBEVDEV_LOG_ERROR:
		prefix = "libevdev error";
		break;
	case LIBEVDEV_LOG_INFO:
		prefix = "libevdev info";
		break;
	case LIBEVDEV_LOG_DEBUG:
		prefix = "libevdev debug";
		break;
	default:
		prefix = "libevdev INVALID LOG PRIORITY";
		break;
	}
	/* default logging format:
	   libevev error in libevdev_some_func: blah blah
	   libevev info in libevdev_some_func: blah blah
	   libevev debug in file.c:123:libevdev_some_func: blah blah
	 */

	fprintf(stderr, "%s in ", prefix);
	if (priority == LIBEVDEV_LOG_DEBUG)
		fprintf(stderr, "%s:%d:", file, line);
	fprintf(stderr, "%s: ", func);
	vfprintf(stderr, format, args);
}

static void fix_invalid_absinfo(const struct libevdev* dev, int axis,
								struct input_absinfo* abs_info) {
	/*
	 * The reported absinfo for ABS_MT_TRACKING_ID is sometimes
	 * uninitialized for certain mtk-soc, due to init code mangling
	 * in the vendor kernel.
	 */
	if (axis == ABS_MT_TRACKING_ID && abs_info->maximum == abs_info->minimum) {
		abs_info->minimum = -1;
		abs_info->maximum = 0xFFFF;
		log_bug(dev, "Device \"%s\" has invalid ABS_MT_TRACKING_ID range",
				dev->name);
	}
}

/*
 * Global logging settings.
 */
static struct logdata log_data = {
	.priority = LIBEVDEV_LOG_INFO,
	.global_handler = libevdev_dflt_log_func,
	.userdata = NULL,
};

void _libevdev_log_msg(const struct libevdev* dev,
					   enum libevdev_log_priority priority, const char* file,
					   int line, const char* func, const char* format, ...) {
	va_list args;

	va_start(args, format);
	log_data.global_handler(priority, log_data.userdata, file, line, func,
							format, args);
	va_end(args);
}

static void libevdev_reset(struct libevdev* dev) {
	enum libevdev_log_priority pri = dev->log.priority;

	free(dev->name);
	free(dev->phys);
	free(dev->uniq);
	free(dev->mt_slot_vals);
	free(dev->mt_sync.mt_state);
	free(dev->mt_sync.tracking_id_changes);
	free(dev->mt_sync.slot_update);
	memset(dev, 0, sizeof(*dev));
	dev->fd = -1;
	dev->initialized = false;
	dev->num_slots = -1;
	dev->current_slot = -1;
	dev->sync_state = SYNC_NONE;
	dev->log.priority = pri;
	libevdev_enable_event_type(dev, EV_SYN);
}

struct libevdev* libevdev_new(void) {
	struct libevdev* dev;

	dev = calloc(1, sizeof(*dev));
	if (!dev)
		return NULL;

	libevdev_reset(dev);

	return dev;
}

void libevdev_free(struct libevdev* dev) {
	if (!dev)
		return;

	queue_free(dev);
	libevdev_reset(dev);
	free(dev);
}

int libevdev_change_fd(struct libevdev* dev, int fd) {
	if (!dev->initialized) {
		log_bug(dev, "device not initialized. call libevdev_set_fd() first\n");
		return -1;
	}
	dev->fd = fd;
	return 0;
}

int libevdev_set_fd(struct libevdev* dev, int fd) {
	int rc;
	int i;
	char buf[256];

	if (dev->initialized) {
		log_bug(dev, "device already initialized.\n");
		return -EBADF;
	} else if (fd < 0)
		return -EBADF;

	libevdev_reset(dev);

	rc = ioctl(fd, EVIOCGBIT(0, sizeof(dev->bits)), dev->bits);
	if (rc < 0)
		goto out;

	memset(buf, 0, sizeof(buf));
	rc = ioctl(fd, EVIOCGNAME(sizeof(buf) - 1), buf);
	if (rc < 0)
		goto out;

	free(dev->name);
	dev->name = strdup(buf);
	if (!dev->name) {
		errno = ENOMEM;
		goto out;
	}

	free(dev->phys);
	dev->phys = NULL;
	memset(buf, 0, sizeof(buf));
	rc = ioctl(fd, EVIOCGPHYS(sizeof(buf) - 1), buf);
	if (rc < 0) {
		/* uinput has no phys */
		if (errno != ENOENT)
			goto out;
	} else {
		dev->phys = strdup(buf);
		if (!dev->phys) {
			errno = ENOMEM;
			goto out;
		}
	}

	free(dev->uniq);
	dev->uniq = NULL;
	memset(buf, 0, sizeof(buf));
	rc = ioctl(fd, EVIOCGUNIQ(sizeof(buf) - 1), buf);
	if (rc < 0) {
		if (errno != ENOENT)
			goto out;
	} else {
		dev->uniq = strdup(buf);
		if (!dev->uniq) {
			errno = ENOMEM;
			goto out;
		}
	}

	rc = ioctl(fd, EVIOCGID, &dev->ids);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGVERSION, &dev->driver_version);
	if (rc < 0)
		goto out;

	/* Built on a kernel with props, running against a kernel without property
	   support. This should not be a fatal case, we'll be missing properties but
	   other than that everything is as expected.
	 */
	rc = ioctl(fd, EVIOCGPROP(sizeof(dev->props)), dev->props);
	if (rc < 0 && errno != EINVAL)
		goto out;

	rc = ioctl(fd, EVIOCGBIT(EV_REL, sizeof(dev->rel_bits)), dev->rel_bits);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(dev->abs_bits)), dev->abs_bits);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGBIT(EV_LED, sizeof(dev->led_bits)), dev->led_bits);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(dev->key_bits)), dev->key_bits);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGBIT(EV_SW, sizeof(dev->sw_bits)), dev->sw_bits);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGBIT(EV_MSC, sizeof(dev->msc_bits)), dev->msc_bits);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGBIT(EV_FF, sizeof(dev->ff_bits)), dev->ff_bits);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGBIT(EV_SND, sizeof(dev->snd_bits)), dev->snd_bits);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGKEY(sizeof(dev->key_values)), dev->key_values);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGLED(sizeof(dev->led_values)), dev->led_values);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGSW(sizeof(dev->sw_values)), dev->sw_values);
	if (rc < 0)
		goto out;

	/* rep is a special case, always set it to 1 for both values if EV_REP is
	 * set
	 */
	if (bit_is_set(dev->bits, EV_REP)) {
		for (i = 0; i < REP_CNT; i++)
			set_bit(dev->rep_bits, i);
		rc = ioctl(fd, EVIOCGREP, dev->rep_values);
		if (rc < 0)
			goto out;
	}

	for (i = ABS_X; i <= ABS_MAX; i++) {
		if (bit_is_set(dev->abs_bits, i)) {
			struct input_absinfo abs_info;
			rc = ioctl(fd, EVIOCGABS(i), &abs_info);
			if (rc < 0)
				goto out;

			fix_invalid_absinfo(dev, i, &abs_info);

			dev->abs_info[i] = abs_info;
		}
	}

	dev->fd = fd;

	/* devices with ABS_MT_SLOT - 1 aren't MT devices,
	   see the documentation for multitouch-related
	   functions for more details */
	if (!libevdev_has_event_code(dev, EV_ABS, ABS_MT_SLOT - 1) &&
		libevdev_has_event_code(dev, EV_ABS, ABS_MT_SLOT)) {
		const struct input_absinfo* abs_info;

		abs_info = libevdev_get_abs_info(dev, ABS_MT_SLOT);

		dev->num_slots = abs_info->maximum + 1;
		dev->mt_slot_vals = calloc(dev->num_slots * ABS_MT_CNT, sizeof(int));
		if (!dev->mt_slot_vals) {
			rc = -ENOMEM;
			goto out;
		}
		dev->current_slot = abs_info->value;

		dev->mt_sync.mt_state_sz =
			sizeof(*dev->mt_sync.mt_state) + (dev->num_slots) * sizeof(int);
		dev->mt_sync.mt_state = calloc(1, dev->mt_sync.mt_state_sz);

		dev->mt_sync.tracking_id_changes_sz =
			NLONGS(dev->num_slots) * sizeof(long);
		dev->mt_sync.tracking_id_changes =
			malloc(dev->mt_sync.tracking_id_changes_sz);

		dev->mt_sync.slot_update_sz =
			NLONGS(dev->num_slots * ABS_MT_CNT) * sizeof(long);
		dev->mt_sync.slot_update = malloc(dev->mt_sync.slot_update_sz);

		if (!dev->mt_sync.tracking_id_changes || !dev->mt_sync.slot_update ||
			!dev->mt_sync.mt_state) {
			rc = -ENOMEM;
			goto out;
		}

		sync_mt_state(dev, 0);
	}

	rc = init_event_queue(dev);
	if (rc < 0) {
		dev->fd = -1;
		return -rc;
	}

	/* not copying key state because we won't know when we'll start to
	 * use this fd and key's are likely to change state by then.
	 * Same with the valuators, really, but they may not change.
	 */

	dev->initialized = true;
out:
	if (rc)
		libevdev_reset(dev);
	return rc ? -errno : 0;
}

int libevdev_get_fd(const struct libevdev* dev) { return dev->fd; }

static inline void init_event(struct libevdev* dev, struct input_event* ev,
							  int type, int code, int value) {
	ev->input_event_sec = dev->last_event_time.tv_sec;
	ev->input_event_usec = dev->last_event_time.tv_usec;
	ev->type = type;
	ev->code = code;
	ev->value = value;
}

static int sync_key_state(struct libevdev* dev) {
	int rc;
	int i;
	unsigned long keystate[NLONGS(KEY_CNT)] = {0};

	rc = ioctl(dev->fd, EVIOCGKEY(sizeof(keystate)), keystate);
	if (rc < 0)
		goto out;

	for (i = 0; i < KEY_CNT; i++) {
		int old, new;
		old = bit_is_set(dev->key_values, i);
		new = bit_is_set(keystate, i);
		if (old ^ new) {
			struct input_event* ev = queue_push(dev);
			init_event(dev, ev, EV_KEY, i, new ? 1 : 0);
		}
	}

	memcpy(dev->key_values, keystate, rc);

	rc = 0;
out:
	return rc ? -errno : 0;
}

static int sync_abs_state(struct libevdev* dev) {
	int rc;
	int i;

	for (i = ABS_X; i < ABS_CNT; i++) {
		struct input_absinfo abs_info;

		if (i >= ABS_MT_MIN && i <= ABS_MT_MAX)
			continue;

		if (!bit_is_set(dev->abs_bits, i))
			continue;

		rc = ioctl(dev->fd, EVIOCGABS(i), &abs_info);
		if (rc < 0)
			goto out;

		if (dev->abs_info[i].value != abs_info.value) {
			struct input_event* ev = queue_push(dev);

			init_event(dev, ev, EV_ABS, i, abs_info.value);
			dev->abs_info[i].value = abs_info.value;
		}
	}

	rc = 0;
out:
	return rc ? -errno : 0;
}

static int sync_mt_state(struct libevdev* dev, int create_events) {
	struct input_event* ev;
	struct input_absinfo abs_info;
	int rc;
	int axis, slot;
	int ioctl_success = 0;
	int last_reported_slot = 0;
	struct mt_sync_state* mt_state = dev->mt_sync.mt_state;
	unsigned long* slot_update = dev->mt_sync.slot_update;
	unsigned long* tracking_id_changes = dev->mt_sync.tracking_id_changes;
	int need_tracking_id_changes = 0;

	memset(dev->mt_sync.slot_update, 0, dev->mt_sync.slot_update_sz);
	memset(dev->mt_sync.tracking_id_changes, 0,
		   dev->mt_sync.tracking_id_changes_sz);

#define AXISBIT(_slot, _axis) (_slot * ABS_MT_CNT + _axis - ABS_MT_MIN)

	for (axis = ABS_MT_MIN; axis <= ABS_MT_MAX; axis++) {
		if (axis == ABS_MT_SLOT)
			continue;

		if (!libevdev_has_event_code(dev, EV_ABS, axis))
			continue;

		mt_state->code = axis;
		rc = ioctl(dev->fd, EVIOCGMTSLOTS(dev->mt_sync.mt_state_sz), mt_state);
		if (rc < 0) {
			/* if the first ioctl fails with -EINVAL, chances are the kernel
			   doesn't support the ioctl. Simply continue */
			if (errno == -EINVAL && !ioctl_success) {
				rc = 0;
			} else /* if the second, ... ioctl fails, really fail */
				goto out;
		} else {
			if (ioctl_success == 0)
				ioctl_success = 1;

			for (slot = 0; slot < dev->num_slots; slot++) {

				if (*slot_value(dev, slot, axis) == mt_state->val[slot])
					continue;

				if (axis == ABS_MT_TRACKING_ID &&
					*slot_value(dev, slot, axis) != -1 &&
					mt_state->val[slot] != -1) {
					set_bit(tracking_id_changes, slot);
					need_tracking_id_changes = 1;
				}

				*slot_value(dev, slot, axis) = mt_state->val[slot];

				set_bit(slot_update, AXISBIT(slot, axis));
				/* note that this slot has updates */
				set_bit(slot_update, AXISBIT(slot, ABS_MT_SLOT));
			}
		}
	}

	if (!create_events) {
		rc = 0;
		goto out;
	}

	if (need_tracking_id_changes) {
		for (slot = 0; slot < dev->num_slots; slot++) {
			if (!bit_is_set(tracking_id_changes, slot))
				continue;

			ev = queue_push(dev);
			init_event(dev, ev, EV_ABS, ABS_MT_SLOT, slot);
			ev = queue_push(dev);
			init_event(dev, ev, EV_ABS, ABS_MT_TRACKING_ID, -1);

			last_reported_slot = slot;
		}

		ev = queue_push(dev);
		init_event(dev, ev, EV_SYN, SYN_REPORT, 0);
	}

	for (slot = 0; slot < dev->num_slots; slot++) {
		if (!bit_is_set(slot_update, AXISBIT(slot, ABS_MT_SLOT)))
			continue;

		ev = queue_push(dev);
		init_event(dev, ev, EV_ABS, ABS_MT_SLOT, slot);
		last_reported_slot = slot;

		for (axis = ABS_MT_MIN; axis <= ABS_MT_MAX; axis++) {
			if (axis == ABS_MT_SLOT ||
				!libevdev_has_event_code(dev, EV_ABS, axis))
				continue;

			if (bit_is_set(slot_update, AXISBIT(slot, axis))) {
				ev = queue_push(dev);
				init_event(dev, ev, EV_ABS, axis, *slot_value(dev, slot, axis));
			}
		}
	}

	/* add one last slot event to make sure the client is on the same
	   slot as the kernel */

	rc = ioctl(dev->fd, EVIOCGABS(ABS_MT_SLOT), &abs_info);
	if (rc < 0)
		goto out;

	dev->current_slot = abs_info.value;

	if (dev->current_slot != last_reported_slot) {
		ev = queue_push(dev);
		init_event(dev, ev, EV_ABS, ABS_MT_SLOT, dev->current_slot);
	}

#undef AXISBIT

	rc = 0;
out:
	return rc ? -errno : 0;
}

static int read_more_events(struct libevdev* dev) {
	int free_elem;
	int len;
	struct input_event* next;

	free_elem = queue_num_free_elements(dev);
	if (free_elem <= 0)
		return 0;

	next = queue_next_element(dev);
	len = read(dev->fd, next, free_elem * sizeof(struct input_event));
	if (len < 0) {
		return -errno;
	} else if (len > 0 && len % sizeof(struct input_event) != 0)
		return -EINVAL;
	else if (len > 0) {
		int nev = len / sizeof(struct input_event);
		queue_set_num_elements(dev, queue_num_elements(dev) + nev);
	}

	return 0;
}

static inline void drain_events(struct libevdev* dev) {
	int rc;
	size_t nelem;
	int iterations = 0;
	const int max_iterations = 8; /* EVDEV_BUF_PACKETS in
									 kernel/drivers/input/evedev.c */

	queue_shift_multiple(dev, queue_num_elements(dev), NULL);

	do {
		rc = read_more_events(dev);
		if (rc == -EAGAIN)
			return;

		if (rc < 0) {
			log_error(dev, "Failed to drain events before sync.\n");
			return;
		}

		nelem = queue_num_elements(dev);
		queue_shift_multiple(dev, nelem, NULL);
	} while (iterations++ < max_iterations && nelem >= queue_size(dev));

	/* Our buffer should be roughly the same or bigger than the kernel
	   buffer in most cases, so we usually don't expect to recurse. If
	   we do, make sure we stop after max_iterations and proceed with
	   what we have.  This could happen if events queue up faster than
	   we can drain them.
	 */
	if (iterations >= max_iterations)
		log_info(dev, "Unable to drain events, buffer size mismatch.\n");
}

static int sync_state(struct libevdev* dev) {
	int rc = 0;
	struct input_event* ev;

	/* see section "Discarding events before synchronizing" in
	 * libevdev/libevdev.h */
	drain_events(dev);

	if (libevdev_has_event_type(dev, EV_KEY))
		rc = sync_key_state(dev);
	if (rc == 0 && libevdev_has_event_type(dev, EV_ABS))
		rc = sync_abs_state(dev);
	if (rc == 0 && dev->num_slots > -1 &&
		libevdev_has_event_code(dev, EV_ABS, ABS_MT_SLOT))
		rc = sync_mt_state(dev, 1);

	dev->queue_nsync = queue_num_elements(dev);

	if (dev->queue_nsync > 0) {
		ev = queue_push(dev);
		init_event(dev, ev, EV_SYN, SYN_REPORT, 0);
		dev->queue_nsync++;
	}

	return rc;
}

static int update_key_state(struct libevdev* dev, const struct input_event* e) {
	if (!libevdev_has_event_type(dev, EV_KEY))
		return 1;

	if (e->code > KEY_MAX)
		return 1;

	set_bit_state(dev->key_values, e->code, e->value != 0);

	return 0;
}

static int update_mt_state(struct libevdev* dev, const struct input_event* e) {
	if (e->code == ABS_MT_SLOT && dev->num_slots > -1) {
		int i;
		dev->current_slot = e->value;
		/* sync abs_info with the current slot values */
		for (i = ABS_MT_SLOT + 1; i <= ABS_MT_MAX; i++) {
			if (libevdev_has_event_code(dev, EV_ABS, i))
				dev->abs_info[i].value = *slot_value(dev, dev->current_slot, i);
		}

		return 0;
	} else if (dev->current_slot == -1)
		return 1;

	*slot_value(dev, dev->current_slot, e->code) = e->value;

	return 0;
}

static int update_abs_state(struct libevdev* dev, const struct input_event* e) {
	if (!libevdev_has_event_type(dev, EV_ABS))
		return 1;

	if (e->code > ABS_MAX)
		return 1;

	if (e->code >= ABS_MT_MIN && e->code <= ABS_MT_MAX)
		update_mt_state(dev, e);

	dev->abs_info[e->code].value = e->value;

	return 0;
}

static int update_state(struct libevdev* dev, const struct input_event* e) {
	int rc = 0;

	switch (e->type) {
	case EV_SYN:
	case EV_REL:
		break;
	case EV_KEY:
		rc = update_key_state(dev, e);
		break;
	case EV_ABS:
		rc = update_abs_state(dev, e);
		break;
	}

	dev->last_event_time.tv_sec = e->input_event_sec;
	dev->last_event_time.tv_usec = e->input_event_usec;

	return rc;
}

/**
 * Sanitize/modify events where needed.
 */
static inline enum event_filter_status
sanitize_event(const struct libevdev* dev, struct input_event* ev,
			   enum SyncState sync_state) {
	if (!libevdev_has_event_code(dev, ev->type, ev->code))
		return EVENT_FILTER_DISCARD;

	if (unlikely(dev->num_slots > -1 &&
				 libevdev_event_is_code(ev, EV_ABS, ABS_MT_SLOT) &&
				 (ev->value < 0 || ev->value >= dev->num_slots))) {
		log_bug(dev,
				"Device \"%s\" received an invalid slot index %d."
				"Capping to announced max slot number %d.\n",
				dev->name, ev->value, dev->num_slots - 1);
		ev->value = dev->num_slots - 1;
		return EVENT_FILTER_MODIFIED;

		/* Drop any invalid tracking IDs, they are only supposed to go from
		   N to -1 or from -1 to N. Never from -1 to -1, or N to M. Very
		   unlikely to ever happen from a real device.
		   */
	} else if (unlikely(
				   sync_state == SYNC_NONE && dev->num_slots > -1 &&
				   libevdev_event_is_code(ev, EV_ABS, ABS_MT_TRACKING_ID) &&
				   ((ev->value == -1 &&
					 *slot_value(dev, dev->current_slot, ABS_MT_TRACKING_ID) ==
						 -1) ||
					(ev->value != -1 &&
					 *slot_value(dev, dev->current_slot, ABS_MT_TRACKING_ID) !=
						 -1)))) {
		log_bug(dev,
				"Device \"%s\" received a double tracking ID %d in slot %d.\n",
				dev->name, ev->value, dev->current_slot);
		return EVENT_FILTER_DISCARD;
	}

	return EVENT_FILTER_NONE;
}

int libevdev_next_event(struct libevdev* dev, unsigned int flags,
						struct input_event* ev) {
	int rc = LIBEVDEV_READ_STATUS_SUCCESS;
	enum event_filter_status filter_status;
	const unsigned int valid_flags =
		LIBEVDEV_READ_FLAG_NORMAL | LIBEVDEV_READ_FLAG_SYNC |
		LIBEVDEV_READ_FLAG_FORCE_SYNC | LIBEVDEV_READ_FLAG_BLOCKING;

	if (!dev->initialized) {
		log_bug(dev, "device not initialized. call libevdev_set_fd() first\n");
		return -EBADF;
	} else if (dev->fd < 0)
		return -EBADF;

	if ((flags & valid_flags) == 0) {
		log_bug(dev, "invalid flags %#x.\n", flags);
		return -EINVAL;
	}

	if (flags & LIBEVDEV_READ_FLAG_SYNC) {
		if (dev->sync_state == SYNC_NEEDED) {
			rc = sync_state(dev);
			if (rc != 0)
				return rc;
			dev->sync_state = SYNC_IN_PROGRESS;
		}

		if (dev->queue_nsync == 0) {
			dev->sync_state = SYNC_NONE;
			return -EAGAIN;
		}

	} else if (dev->sync_state != SYNC_NONE) {
		struct input_event e;

		/* call update_state for all events here, otherwise the library has the
		   wrong view of the device too */
		while (queue_shift(dev, &e) == 0) {
			dev->queue_nsync--;
			if (sanitize_event(dev, &e, dev->sync_state) !=
				EVENT_FILTER_DISCARD)
				update_state(dev, &e);
		}

		dev->sync_state = SYNC_NONE;
	}

	/* Always read in some more events. Best case this smoothes over a potential
	   SYN_DROPPED, worst case we don't read fast enough and end up with
	   SYN_DROPPED anyway.

	   Except if the fd is in blocking mode and we still have events from the
	   last read, don't read in any more.
	 */
	do {
		if (!(flags & LIBEVDEV_READ_FLAG_BLOCKING) ||
			queue_num_elements(dev) == 0) {
			rc = read_more_events(dev);
			if (rc < 0 && rc != -EAGAIN)
				goto out;
		}

		if (flags & LIBEVDEV_READ_FLAG_FORCE_SYNC) {
			dev->sync_state = SYNC_NEEDED;
			rc = LIBEVDEV_READ_STATUS_SYNC;
			goto out;
		}

		if (queue_shift(dev, ev) != 0)
			return -EAGAIN;

		filter_status = sanitize_event(dev, ev, dev->sync_state);
		if (filter_status != EVENT_FILTER_DISCARD)
			update_state(dev, ev);

		/* if we disabled a code, get the next event instead */
	} while (filter_status == EVENT_FILTER_DISCARD ||
			 !libevdev_has_event_code(dev, ev->type, ev->code));

	rc = LIBEVDEV_READ_STATUS_SUCCESS;
	if (ev->type == EV_SYN && ev->code == SYN_DROPPED) {
		dev->sync_state = SYNC_NEEDED;
		rc = LIBEVDEV_READ_STATUS_SYNC;
	}

	if (flags & LIBEVDEV_READ_FLAG_SYNC && dev->queue_nsync > 0) {
		dev->queue_nsync--;
		rc = LIBEVDEV_READ_STATUS_SYNC;
		if (dev->queue_nsync == 0) {
			struct input_event next;
			dev->sync_state = SYNC_NONE;

			if (queue_peek(dev, 0, &next) == 0 && next.type == EV_SYN &&
				next.code == SYN_DROPPED)
				log_info(dev, "SYN_DROPPED received after finished "
							  "sync - you're not keeping up\n");
		}
	}

out:
	return rc;
}

int libevdev_has_event_pending(struct libevdev* dev) {
	struct pollfd fds = {dev->fd, POLLIN, 0};
	int rc;

	if (!dev->initialized) {
		log_bug(dev, "device not initialized. call libevdev_set_fd() first\n");
		return -EBADF;
	} else if (dev->fd < 0)
		return -EBADF;

	if (queue_num_elements(dev) != 0)
		return 1;

	rc = poll(&fds, 1, 0);
	return (rc >= 0) ? rc : -errno;
}

const char* libevdev_get_name(const struct libevdev* dev) {
	return dev->name ? dev->name : "";
}

int libevdev_has_event_type(const struct libevdev* dev, unsigned int type) {
	return type == EV_SYN || (type <= EV_MAX && bit_is_set(dev->bits, type));
}

int libevdev_has_event_code(const struct libevdev* dev, unsigned int type,
							unsigned int code) {
	const unsigned long* mask = NULL;
	int max;

	if (!libevdev_has_event_type(dev, type))
		return 0;

	if (type == EV_SYN)
		return 1;

	max = type_to_mask_const(dev, type, &mask);

	if (max == -1 || code > (unsigned int)max)
		return 0;

	return bit_is_set(mask, code);
}

int libevdev_get_num_slots(const struct libevdev* dev) {
	return dev->num_slots;
}

const struct input_absinfo* libevdev_get_abs_info(const struct libevdev* dev,
												  unsigned int code) {
	if (!libevdev_has_event_type(dev, EV_ABS) ||
		!libevdev_has_event_code(dev, EV_ABS, code))
		return NULL;

	return &dev->abs_info[code];
}

int libevdev_enable_event_type(struct libevdev* dev, unsigned int type) {
	int max;

	if (type > EV_MAX)
		return -1;

	if (libevdev_has_event_type(dev, type))
		return 0;

	max = libevdev_event_type_get_max(type);
	if (max == -1)
		return -1;

	set_bit(dev->bits, type);

	if (type == EV_REP) {
		int delay = 0, period = 0;
		libevdev_enable_event_code(dev, EV_REP, REP_DELAY, &delay);
		libevdev_enable_event_code(dev, EV_REP, REP_PERIOD, &period);
	}
	return 0;
}

int libevdev_enable_event_code(struct libevdev* dev, unsigned int type,
							   unsigned int code, const void* data) {
	unsigned int max;
	unsigned long* mask = NULL;

	if (libevdev_enable_event_type(dev, type))
		return -1;

	switch (type) {
	case EV_SYN:
		return 0;
	case EV_ABS:
	case EV_REP:
		if (data == NULL)
			return -1;
		break;
	default:
		if (data != NULL)
			return -1;
		break;
	}

	max = type_to_mask(dev, type, &mask);

	if (code > max || (int)max == -1)
		return -1;

	set_bit(mask, code);

	if (type == EV_ABS) {
		const struct input_absinfo* abs = data;
		dev->abs_info[code] = *abs;
	} else if (type == EV_REP) {
		const int* value = data;
		dev->rep_values[code] = *value;
	}

	return 0;
}

int libevdev_event_is_type(const struct input_event* ev, unsigned int type) {
	return type < EV_CNT && ev->type == type;
}

int libevdev_event_is_code(const struct input_event* ev, unsigned int type,
						   unsigned int code) {
	int max;

	if (!libevdev_event_is_type(ev, type))
		return 0;

	max = libevdev_event_type_get_max(type);
	return (max > -1 && code <= (unsigned int)max && ev->code == code);
}

#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winitializer-overrides"
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
#endif
static const int ev_max[EV_MAX + 1] = {
	[0 ... EV_MAX] = -1, [EV_REL] = REL_MAX, [EV_ABS] = ABS_MAX,
	[EV_KEY] = KEY_MAX,	 [EV_LED] = LED_MAX, [EV_SND] = SND_MAX,
	[EV_MSC] = MSC_MAX,	 [EV_SW] = SW_MAX,	 [EV_FF] = FF_MAX,
	[EV_SYN] = SYN_MAX,	 [EV_REP] = REP_MAX,
};
#if __clang__
#pragma clang diagnostic pop /* "-Winitializer-overrides" */
#else
#pragma GCC diagnostic pop /* "-Woverride-init" */
#endif

int libevdev_event_type_get_max(unsigned int type) {
	if (type > EV_MAX)
		return -1;

	return ev_max[type];
}
