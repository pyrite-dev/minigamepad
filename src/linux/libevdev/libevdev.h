// /*
//  * Copyright © 2013 Red Hat, Inc.
//  *
//  * Permission to use, copy, modify, distribute, and sell this software and
//  its
//  * documentation for any purpose is hereby granted without fee, provided that
//  * the above copyright notice appear in all copies and that both that
//  copyright
//  * notice and this permission notice appear in supporting documentation, and
//  * that the name of the copyright holders not be used in advertising or
//  * publicity pertaining to distribution of the software without specific,
//  * written prior permission.  The copyright holders make no representations
//  * about the suitability of this software for any purpose.  It is provided
//  "as
//  * is" without express or implied warranty.
//  *
//  * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
//  SOFTWARE,
//  * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
//  * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
//  * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
//  USE,
//  * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
//  * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
//  PERFORMANCE
//  * OF THIS SOFTWARE.
//  */

#ifndef LIBEVDEV_H
#define LIBEVDEV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/input.h>
#include <stdarg.h>

#define LIBEVDEV_ATTRIBUTE_PRINTF(_format, _args)                              \
	__attribute__((format(printf, _format, _args)))

/**
 * @ingroup init
 *
 * Opaque struct representing an evdev device.
 */
struct libevdev;

enum libevdev_read_flag {
	LIBEVDEV_READ_FLAG_SYNC = 1,
	LIBEVDEV_READ_FLAG_NORMAL = 2,
	LIBEVDEV_READ_FLAG_FORCE_SYNC = 4,
	LIBEVDEV_READ_FLAG_BLOCKING = 8
};

struct libevdev* libevdev_new(void);

void libevdev_free(struct libevdev* dev);

enum libevdev_log_priority {
	LIBEVDEV_LOG_ERROR = 10, /**< critical errors and application bugs */
	LIBEVDEV_LOG_INFO = 20,	 /**< informational messages */
	LIBEVDEV_LOG_DEBUG = 30	 /**< debug information */
};
int libevdev_set_fd(struct libevdev* dev, int fd);
int libevdev_get_fd(const struct libevdev* dev);
enum libevdev_read_status {
	/**
	 * libevdev_next_event() has finished without an error
	 * and an event is available for processing.
	 *
	 * @see libevdev_next_event
	 */
	LIBEVDEV_READ_STATUS_SUCCESS = 0,
	/**
	 * Depending on the libevdev_next_event() read flag:
	 * * libevdev received a SYN_DROPPED from the device, and the caller should
	 * now resync the device, or,
	 * * an event has been read in sync mode.
	 *
	 * @see libevdev_next_event
	 */
	LIBEVDEV_READ_STATUS_SYNC = 1
};
int libevdev_next_event(struct libevdev* dev, unsigned int flags,
						struct input_event* ev);
int libevdev_has_event_pending(struct libevdev* dev);
const char* libevdev_get_name(const struct libevdev* dev);
int libevdev_has_event_type(const struct libevdev* dev, unsigned int type);
int libevdev_has_event_code(const struct libevdev* dev, unsigned int type,
							unsigned int code);
const struct input_absinfo* libevdev_get_abs_info(const struct libevdev* dev,
												  unsigned int code);
int libevdev_get_num_slots(const struct libevdev* dev);
int libevdev_enable_event_type(struct libevdev* dev, unsigned int type);
int libevdev_enable_event_code(struct libevdev* dev, unsigned int type,
							   unsigned int code, const void* data);
int libevdev_event_is_code(const struct input_event* ev, unsigned int type,
						   unsigned int code);
int libevdev_event_type_get_max(unsigned int type);
typedef void (*libevdev_log_func_t)(enum libevdev_log_priority priority,
									void* data, const char* file, int line,
									const char* func, const char* format,
									va_list args)
	LIBEVDEV_ATTRIBUTE_PRINTF(6, 0);

#ifdef __cplusplus
}
#endif

#endif /* LIBEVDEV_H */
