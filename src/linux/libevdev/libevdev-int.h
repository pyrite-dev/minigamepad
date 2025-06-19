/*
 * Copyright © 2013 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided
 "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifndef LIBEVDEV_INT_H
#define LIBEVDEV_INT_H

#include "libevdev-util.h"
#include "libevdev.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME 256
#define ALIAS(_to) __attribute__((alias(#_to)))

enum SyncState {
	SYNC_NONE,
	SYNC_NEEDED,
	SYNC_IN_PROGRESS,
};

struct logdata {
	enum libevdev_log_priority priority; /** minimum logging priority */
	libevdev_log_func_t global_handler;	 /** global handler function */
	void* userdata;						 /** user-defined data pointer */
};

struct libevdev {
	int fd;
	bool initialized;
	char name[256];
	struct input_id ids;
	int driver_version;
	unsigned long bits[NLONGS(EV_CNT)];
	unsigned long props[NLONGS(INPUT_PROP_CNT)];
	unsigned long key_bits[NLONGS(KEY_CNT)];
	unsigned long abs_bits[NLONGS(ABS_CNT)];
	unsigned long ff_bits[NLONGS(FF_CNT)];
	unsigned long key_values[NLONGS(KEY_CNT)];
	struct input_absinfo abs_info[ABS_CNT];
	int rep_values[REP_CNT];

	enum SyncState sync_state;

	struct input_event* queue;
	size_t queue_size;	/**< size of queue in elements */
	size_t queue_next;	/**< next event index */
	size_t queue_nsync; /**< number of sync events */

	struct timeval last_event_time;

	struct logdata log;
};

static inline struct input_event* queue_push(struct libevdev* dev) {
	if (dev->queue_next >= dev->queue_size)
		return NULL;

	return &dev->queue[dev->queue_next++];
}

static inline int queue_shift_multiple(struct libevdev* dev, size_t n,
									   struct input_event* ev) {
	size_t remaining;

	if (dev->queue_next == 0)
		return 0;

	remaining = dev->queue_next;
	n = min(n, remaining);
	remaining -= n;

	if (ev)
		memcpy(ev, dev->queue, n * sizeof(*ev));

	memmove(dev->queue, &dev->queue[n], remaining * sizeof(*dev->queue));

	dev->queue_next = remaining;
	return n;
}

/**
 * Set ev to the first element in the queue, shifting everything else
 * forward by one.
 *
 * @return 0 on success, 1 if the queue is empty.
 */
static inline int queue_shift(struct libevdev* dev, struct input_event* ev) {
	return queue_shift_multiple(dev, 1, ev) == 1 ? 0 : 1;
}

static inline int queue_alloc(struct libevdev* dev, size_t size) {
	if (size == 0)
		return -ENOMEM;

	dev->queue = calloc(size, sizeof(struct input_event));
	if (!dev->queue)
		return -ENOMEM;

	dev->queue_size = size;
	dev->queue_next = 0;
	return 0;
}

static inline void queue_free(struct libevdev* dev) {
	free(dev->queue);
	dev->queue_size = 0;
	dev->queue_next = 0;
}

static inline size_t queue_num_elements(struct libevdev* dev) {
	return dev->queue_next;
}

static inline size_t queue_size(struct libevdev* dev) {
	return dev->queue_size;
}

static inline size_t queue_num_free_elements(struct libevdev* dev) {
	if (dev->queue_size == 0)
		return 0;

	return dev->queue_size - dev->queue_next;
}

static inline struct input_event* queue_next_element(struct libevdev* dev) {
	if (dev->queue_next == dev->queue_size)
		return NULL;

	return &dev->queue[dev->queue_next];
}

static inline int queue_set_num_elements(struct libevdev* dev, size_t nelem) {
	if (nelem > dev->queue_size)
		return 1;

	dev->queue_next = nelem;

	return 0;
}

#endif
