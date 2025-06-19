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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libevdev-int.h"
#include "libevdev-util.h"
#include "libevdev.h"

#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winitializer-overrides"
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
#endif
static const int ev_max[EV_MAX + 1] = {
	[0 ... EV_MAX] = -1,
	[EV_ABS] = ABS_MAX,
	[EV_KEY] = KEY_MAX,
	[EV_FF] = FF_MAX,
};
#if __clang__
#pragma clang diagnostic pop /* "-Winitializer-overrides" */
#else
#pragma GCC diagnostic pop /* "-Woverride-init" */
#endif

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
		int max = ev_max[type];
		for (code = 0; max > 0 && code < (unsigned int)max; code++) {
			if (libevdev_has_event_code(dev, type, code))
				nevents++;
		}
	}

	return queue_alloc(dev, max(MIN_QUEUE_SIZE, nevents * 2));
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
		printf("[BUG] Device \"%s\" has invalid ABS_MT_TRACKING_ID range",
			   dev->name);
	}
}

/*
 * Global logging settings.
 */
static struct logdata log_data = {
	.priority = LIBEVDEV_LOG_INFO,
	.userdata = NULL,
};

static void libevdev_reset(struct libevdev* dev) {
	enum libevdev_log_priority pri = dev->log.priority;

	free(dev->name);
	memset(dev, 0, sizeof(*dev));
	dev->fd = -1;
	dev->initialized = false;
	dev->sync_state = SYNC_NONE;
	dev->log.priority = pri;
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
		printf("[BUG] device not initialized. call libevdev_set_fd() first\n");
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
		printf("[BUG] device already initialized.\n");
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

	rc = ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(dev->abs_bits)), dev->abs_bits);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(dev->key_bits)), dev->key_bits);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGBIT(EV_FF, sizeof(dev->ff_bits)), dev->ff_bits);
	if (rc < 0)
		goto out;

	// rc = ioctl(fd, EVIOCGBIT(EV_SND, sizeof(dev->snd_bits)), dev->snd_bits);
	// if (rc < 0)
	// 	goto out;

	// rc = ioctl(fd, EVIOCGKEY(sizeof(dev->key_values)), dev->key_values);
	// if (rc < 0)
	// 	goto out;

	// rc = ioctl(fd, EVIOCGLED(sizeof(dev->led_values)), dev->led_values);
	// if (rc < 0)
	// 	goto out;

	// rc = ioctl(fd, EVIOCGSW(sizeof(dev->sw_values)), dev->sw_values);
	// if (rc < 0)
	// 	goto out;

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
			printf("[BUG] Failed to drain events before sync.\n");
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
		printf("[BUG] Unable to drain events, buffer size mismatch.\n");
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

static int update_abs_state(struct libevdev* dev, const struct input_event* e) {
	if (!libevdev_has_event_type(dev, EV_ABS))
		return 1;

	if (e->code > ABS_MAX)
		return 1;

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

int libevdev_next_event(struct libevdev* dev, struct input_event* ev) {
	int rc = LIBEVDEV_READ_STATUS_SUCCESS;

	if (!dev->initialized) {
		printf("[BUG] device not initialized. call libevdev_set_fd() first\n");
		return -EBADF;
	} else if (dev->fd < 0)
		return -EBADF;

	if (dev->sync_state != SYNC_NONE) {
		struct input_event e;

		/* call update_state for all events here, otherwise the library has the
		   wrong view of the device too */
		while (queue_shift(dev, &e) == 0) {
			dev->queue_nsync--;
			if (libevdev_has_event_code(dev, ev->type, ev->code))
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
		if (queue_num_elements(dev) == 0) {
			rc = read_more_events(dev);
			if (rc < 0 && rc != -EAGAIN)
				goto out;
		}

		if (queue_shift(dev, ev) != 0)
			return -EAGAIN;

		if (libevdev_has_event_code(dev, ev->type, ev->code))
			update_state(dev, ev);

		/* if we disabled a code, get the next event instead */
	} while (!libevdev_has_event_code(dev, ev->type, ev->code));

	rc = LIBEVDEV_READ_STATUS_SUCCESS;
	if (ev->type == EV_SYN && ev->code == SYN_DROPPED) {
		dev->sync_state = SYNC_NEEDED;
		rc = LIBEVDEV_READ_STATUS_SYNC;
	}

out:
	return rc;
}

int libevdev_has_event_pending(struct libevdev* dev) {
	struct pollfd fds = {dev->fd, POLLIN, 0};
	int rc;

	if (!dev->initialized) {
		printf("[BUG] device not initialized. call libevdev_set_fd() first\n");
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

#define max_mask(uc, lc)                                                       \
	case EV_##uc:                                                              \
		*mask = dev->lc##_bits;                                                \
		max = ev_max[type];                                                    \
		break;

static inline int type_to_mask_const(const struct libevdev* dev,
									 unsigned int type,
									 const unsigned long** mask) {
	int max;

	switch (type) {
		max_mask(ABS, abs);
		max_mask(KEY, key);
		max_mask(FF, ff);
	default:
		max = -1;
		break;
	}

	return max;
}
#undef max_mask

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

int libevdev_event_is_code(const struct input_event* ev, unsigned int type,
						   unsigned int code) {
	int max = ev_max[type];
	return (max > -1 && code <= (unsigned int)max && ev->code == code);
}
