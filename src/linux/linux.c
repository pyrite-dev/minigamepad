#include "linux.h"
#include "libevdev/libevdev.h"
#include "minigamepad.h"
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "sdl_db.h"

#define MG_UNUSED(x) (void)(x)

struct mg_inotify {
	int inotify;
	int watch;
};

struct mg_inotify global = {0, 0};

void mg_gamepads_backend_init(mg_gamepads* gamepads) {
	if (global.inotify == 0) {
		global.inotify = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
		if (global.inotify > 0) {
			// HACK: Register for IN_ATTRIB to get notified when udev is done
			//       This works well in practice but the true way is libudev

			global.watch = inotify_add_watch(global.inotify, "/dev/input/",
											 IN_CREATE | IN_ATTRIB | IN_DELETE);
		}
	}

	struct dirent* dp;
	DIR* dfd;

	// open the directory where all the devices are gonna be
	if ((dfd = opendir("/dev/input/")) == NULL) {
		fprintf(stderr, "Can't open /dev/input/\n");
		return;
	}
	char full_path[273];
	// for each file found:
	while ((dp = readdir(dfd)) != NULL) {
		// get the full path of it (size of path + size of file name)
		snprintf(full_path, sizeof(full_path), "/dev/input/%s", dp->d_name);

		setup_gamepad(gamepads, (char*)full_path);
	}

	closedir(dfd);
}

void mg_gamepads_backend_free(mg_gamepads* gamepads) {
	MG_UNUSED(gamepads);

	if (global.inotify > 0) {
		if (global.watch > 0)
			inotify_rm_watch(global.inotify, global.watch);

		close(global.inotify);
	}

	global = (struct mg_inotify){0, 0};
}

bool setup_gamepad(mg_gamepads* gamepads, char* full_path) {
	mg_gamepad* gamepad = mg_alloc(gamepads);
	if (gamepad == NULL) {
		return false;
	}

	struct mg_gamepad_context_t* ctx =
		malloc(sizeof(struct mg_gamepad_context_t));

	gamepad->ctx = ctx;

	// open said full path with libevdev
	ctx->dev = libevdev_new();

	if (libevdev_set_fd(ctx->dev, open(full_path, O_RDWR))) {
		// char err[256];
		// snprintf(err, 256, "could not open %s", full_path);
		// perror(err);
		mg_gamepad_remove(gamepads, gamepad);
		return false;
	}

	struct input_id id = {0};
	char evBits[(EV_CNT + 7) / 8] = {0};
	char keyBits[(KEY_CNT + 7) / 8] = {0};
	char absBits[(ABS_CNT + 7) / 8] = {0};

	int fd = libevdev_get_fd(gamepad->ctx->dev);
	if (ioctl(fd, EVIOCGBIT(0, sizeof(evBits)), evBits) < 0 ||
		ioctl(fd, EVIOCGID, &id) < 0 ||
		ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keyBits)), keyBits) < 0 ||
		ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absBits)), absBits) < 0) {
		mg_gamepad_remove(gamepads, gamepad);
		return false;
	}

#define isBitSet(bit, arr) (arr[(bit) / 8] & (1 << ((bit) % 8)))
	if (!isBitSet(EV_ABS, evBits)) {
		mg_gamepad_remove(gamepads, gamepad);
		return false;
	}

	int buttonCount = 0, axisCount = 0;
	memset(gamepad->buttons, 0, sizeof(gamepad->buttons));
	memset(gamepad->buttons, 0, sizeof(gamepad->axises));

	// go through any buttons a gamepad would have
	for (unsigned int i = BTN_MISC; i < KEY_CNT; i++) {
		if (!isBitSet(i, keyBits))
			continue;

		gamepad->ctx->keyMap[i - BTN_MISC] = (unsigned int)buttonCount;
		buttonCount++;
	}

	// go through any axises a gamepad would have
	for (unsigned int i = 0; i < ABS_CNT; i++) {
		if (!isBitSet(i, absBits))
			continue;

		if (ioctl(fd, EVIOCGABS(i), &gamepad->ctx->absInfo[i]) < 0)
			continue;

		gamepad->ctx->absMap[i] = (unsigned int)axisCount;
		axisCount++;
	}

	if ((axisCount == 0 && buttonCount == 0) ||
		buttonCount > MG_GAMEPAD_BUTTON_MAX + 10) {
		mg_gamepad_remove(gamepads, gamepad);
		return false;
	}
#undef isBitSet

	memcpy(gamepad->ctx->full_path, full_path, sizeof(gamepad->ctx->full_path));
	// Generate a joystick GUID that matches the SDL 2.0.5+ one (sourced from
	// GLFW)
	const char* name = libevdev_get_name(gamepad->ctx->dev);
	if (id.vendor && id.product && id.version) {
		snprintf(gamepad->guid, sizeof(gamepad->guid),
				 "%02x%02x0000%02x%02x0000%02x%02x0000%02x%02x0000",
				 id.bustype & 0xff, id.bustype >> 8, id.vendor & 0xff,
				 id.vendor >> 8, id.product & 0xff, id.product >> 8,
				 id.version & 0xff, id.version >> 8);
	} else {
		snprintf(gamepad->guid, sizeof(gamepad->guid),
				 "%02x%02x0000%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x00",
				 id.bustype & 0xff, id.bustype >> 8, name[0], name[1], name[2],
				 name[3], name[4], name[5], name[6], name[7], name[8], name[9],
				 name[10]);
	}

	strncpy(gamepad->name, name, sizeof(gamepad->name) - 1);
	gamepad->mapping = mg_gamepad_find_valid_mapping(gamepad);

	for (unsigned int btn = BTN_MISC; btn < KEY_CNT; btn++) {
		mg_gamepad_btn key =
			mg_get_gamepad_btn(gamepad, gamepad->ctx->keyMap[btn - BTN_MISC]);
		if (key == MG_GAMEPAD_BUTTON_UNKNOWN)
			key = mg_get_gamepad_btn_backend(btn);
		if (key == MG_GAMEPAD_BUTTON_UNKNOWN)
			continue;

		if (gamepad->buttons[key].supported)
			continue;

		if (libevdev_has_event_code(ctx->dev, EV_KEY, btn) == false) {
			gamepad->buttons[key].supported = false;
			continue;
		}

		gamepad->buttons[key].supported = true;
		gamepad->buttons[key].value = 0;
	}

	for (unsigned int axis = 0; axis < ABS_CNT; axis++) {
		if (libevdev_has_event_code(ctx->dev, EV_ABS, axis) == false) {
			continue;
		}

		// We want every axis except the hats (which are usually d-pads)
		// to have a deadzone of 5000. The idea is that programmer can override
		// this later, by a config file or something.
		int16_t deadzone = 0;
		switch (axis) {
		case ABS_HAT0X:
			gamepad->buttons[MG_GAMEPAD_BUTTON_DPAD_LEFT].supported = true;
			gamepad->buttons[MG_GAMEPAD_BUTTON_DPAD_LEFT].value = 0;
			gamepad->buttons[MG_GAMEPAD_BUTTON_DPAD_RIGHT].supported = true;
			gamepad->buttons[MG_GAMEPAD_BUTTON_DPAD_RIGHT].value = 0;
			deadzone = 0;
			break;
		case ABS_HAT0Y:
			gamepad->buttons[MG_GAMEPAD_BUTTON_DPAD_UP].supported = true;
			gamepad->buttons[MG_GAMEPAD_BUTTON_DPAD_UP].value = 0;
			gamepad->buttons[MG_GAMEPAD_BUTTON_DPAD_DOWN].supported = true;
			gamepad->buttons[MG_GAMEPAD_BUTTON_DPAD_DOWN].value = 0;
			deadzone = 0;
			break;
		case ABS_HAT1X:
		case ABS_HAT1Y:
		case ABS_HAT2X:
		case ABS_HAT2Y:
		case ABS_HAT3X:
		case ABS_HAT3Y:
			deadzone = 0;
			break;
		case ABS_Z:
			gamepad->buttons[MG_GAMEPAD_BUTTON_LEFT_TRIGGER].supported = true;
			gamepad->buttons[MG_GAMEPAD_BUTTON_LEFT_TRIGGER].value = 0;
			deadzone = 0;
			break;
		case ABS_RZ:
			gamepad->buttons[MG_GAMEPAD_BUTTON_RIGHT_TRIGGER].supported = true;
			gamepad->buttons[MG_GAMEPAD_BUTTON_RIGHT_TRIGGER].value = 0;
			deadzone = 0;
			break;
		default:
			deadzone = 15;
			break;
		}

		mg_gamepad_axis key =
			mg_get_gamepad_axis(gamepad, gamepad->ctx->absMap[axis]);
		if (key == MG_GAMEPAD_AXIS_UNKNOWN)
			key = mg_get_gamepad_axis_backend(axis);
		if (key == MG_GAMEPAD_AXIS_UNKNOWN)
			continue;

		gamepad->axises[key].supported = true;
		gamepad->axises[key].value = 0;
		gamepad->axises[key].deadzone = deadzone;
	}

	if (libevdev_has_event_code(
			ctx->dev, EV_FF,
			FF_RUMBLE)) { // this is a struct that gets passed to the
		// rumble effect when
		// activated. we have to "erase" the effect whenever we want to add a
		// new one, hence this gets put in the struct itself so that we can keep
		// track of the id. and while we're here we save some other values
		// that'll never get changed.
		gamepad->ctx->effect = (struct ff_effect){
			.type = FF_RUMBLE,
			.id = -1,
			.direction = 0,
			.trigger = {0, 0},
			.replay =
				{
					.delay = 0,
				},
		};
		gamepad->ctx->supports_rumble = true;
	} else {
		gamepad->ctx->supports_rumble = false;
	}

	return true;
}

bool mg_gamepads_fetch(mg_gamepads* gamepads) {
	if (global.inotify <= 0)
		return false;

	ssize_t offset = 0;
	char buffer[16384];
	const ssize_t size = read(global.inotify, buffer, sizeof(buffer));

	char full_path[273];
	const char path[] = "/dev/input/";

	bool ret = false;
	while (size > offset) {
		const struct inotify_event* e =
			(struct inotify_event*)(buffer + offset);

		offset += (ssize_t)sizeof(struct inotify_event) + e->len;

		if (strncmp(e->name, "event", 5) != 0) {
			continue;
		}

		snprintf(full_path, sizeof(full_path), "%s%s", path, e->name);
		full_path[272] = '\0';

		if (e->mask & (IN_CREATE | IN_ATTRIB)) {
			if (setup_gamepad(gamepads, full_path)) {
				ret = true;
			}
		}

		else if (e->mask & IN_DELETE) {
			for (mg_gamepad* cur = gamepads->head; cur != NULL;
				 cur = cur->next) {
				if (strncmp(cur->ctx->full_path, full_path,
							sizeof(cur->ctx->full_path)) == 0) {
					mg_gamepad_remove(gamepads, cur);
					break;
				}
			}
		}
	}

	return ret;
}

void mg_gamepad_free(mg_gamepad* gamepad) {
	libevdev_free(gamepad->ctx->dev);
	free(gamepad->ctx);
}

#define emulate_button(button, axis, min, max, flip)                           \
	{                                                                          \
		if ((gamepad->axises[axis].value >= max &&                             \
			 flip !gamepad->buttons[button].value) ||                          \
			(gamepad->axises[axis].value <= min &&                             \
			 flip gamepad->buttons[button].value)) {                           \
			gamepad->buttons[button].value =                                   \
				flip(gamepad->axises[axis].value >= max) ? 1 : 0;              \
			if (event != NULL) {                                               \
				event->gamepad = gamepad;                                      \
				event->btn = button;                                           \
				event->type = flip(gamepad->axises[button].value >= max)       \
								  ? MG_GAMEPAD_BTN_PRESS                       \
								  : MG_GAMEPAD_BTN_RELEASE;                    \
				return true;                                                   \
			}                                                                  \
		}                                                                      \
	}

bool mg_gamepad_update(mg_gamepad* gamepad, mg_gamepad_event* event) {
	if (gamepad->connected == false)
		return false;

	// go through libevdev events.
	emulate_button(MG_GAMEPAD_BUTTON_LEFT_TRIGGER, MG_GAMEPAD_AXIS_LEFT_TRIGGER,
				   -98, 98, )
		emulate_button(MG_GAMEPAD_BUTTON_RIGHT_TRIGGER,
					   MG_GAMEPAD_AXIS_RIGHT_TRIGGER, -98, 98, )

			emulate_button(MG_GAMEPAD_BUTTON_DPAD_LEFT,
						   MG_GAMEPAD_AXIS_HAT_DPAD_LEFT, -100, 0, !)
				emulate_button(MG_GAMEPAD_BUTTON_DPAD_RIGHT,
							   MG_GAMEPAD_AXIS_HAT_DPAD_RIGHT, 0, 100, )
					emulate_button(MG_GAMEPAD_BUTTON_DPAD_UP,
								   MG_GAMEPAD_AXIS_HAT_DPAD_UP, -100, 0, !)
						emulate_button(MG_GAMEPAD_BUTTON_DPAD_DOWN,
									   MG_GAMEPAD_AXIS_HAT_DPAD_DOWN, 0, 100, )

							struct input_event ev;
	do {
		int pending = libevdev_has_event_pending(gamepad->ctx->dev);
		if (!pending) {
			return false;
		}

		int rc = libevdev_next_event(gamepad->ctx->dev, &ev);
		if (rc) {
			return false;
		}
	} while (ev.type != EV_KEY &&
			 ev.type != EV_ABS); // Ignore events we don't handle

	switch (ev.type) {
	case EV_KEY: {
		mg_gamepad_btn btn = mg_get_gamepad_btn(
			gamepad, (unsigned int)(gamepad->ctx->keyMap[ev.code - BTN_MISC]));
		if (btn == MG_GAMEPAD_BUTTON_UNKNOWN) {
			btn = mg_get_gamepad_btn_backend(ev.code);
		}

		if (btn != MG_GAMEPAD_BUTTON_UNKNOWN) {
			gamepad->buttons[btn].value = (int16_t)ev.value;
		}

		if (event != NULL) {
			event->gamepad = gamepad;
			event->btn = btn;
			event->type =
				ev.value ? MG_GAMEPAD_BTN_PRESS : MG_GAMEPAD_BTN_RELEASE;
		}
		return true;
	}
	case EV_ABS: {
		mg_gamepad_axis axis =
			mg_get_gamepad_axis(gamepad, gamepad->ctx->absMap[ev.code]);
		if (axis == MG_GAMEPAD_AXIS_UNKNOWN)
			axis = mg_get_gamepad_axis_backend(ev.code);
		if (axis == MG_GAMEPAD_AXIS_UNKNOWN) {
			event->type = MG_GAMEPAD_AXIS_MOVE;
			event->gamepad = gamepad;
			event->axis = axis;
			return true;
		}

		const struct input_absinfo info = gamepad->ctx->absInfo[ev.code];
		float normalized = (float)ev.value;

		const float range = (float)(info.maximum - info.minimum);
		if (range) {
			// Normalize to 0.0 -> 1.0
			normalized = (normalized - (float)info.minimum) / range;
			// Normalize to -1.0 -> 1.0
			normalized = normalized * 2.0f - 1.0f;
		}

		if (gamepad->axises[axis].supported) {
			int deadzone = gamepad->axises[axis].deadzone;
			int16_t event_val = (int16_t)(normalized * 100);
			if (abs(event_val) < deadzone) {
				event_val = 0;
			}

			gamepad->axises[axis].value = event_val;
		}

		if (event != NULL) {
			event->type = MG_GAMEPAD_AXIS_MOVE;
			event->gamepad = gamepad;
			event->axis = axis;
		}
		return true;
	}
	default:
		break;
	}

	return false;
}

void mg_gamepad_rumble(mg_gamepad* gamepad, uint16_t strong_vibration,
					   uint16_t weak_vibration, uint16_t milliseconds) {
	// only continue if the controller does rumble
	if (!gamepad->ctx->supports_rumble) {
		return;
	}

	// libevdev doesn't support rumble so we have to do it raw.

	// get the fd that libevdev is holding for the input device
	int fd = libevdev_get_fd(gamepad->ctx->dev);

	//  if we currently have an effect going on, erase it to make room for the
	//  new one.
	if (gamepad->ctx->effect.id != -1) {
		if (ioctl(fd, EVIOCRMFF, gamepad->ctx->effect.id) == -1) {
			perror("could not erase rumble");
			return;
		}
		gamepad->ctx->effect.id = -1;
	}

	// configure the effect based on this function's parameters
	gamepad->ctx->effect.replay.length = milliseconds;
	gamepad->ctx->effect.u.rumble.weak_magnitude = weak_vibration;
	gamepad->ctx->effect.u.rumble.strong_magnitude = strong_vibration;

	// upload the effect to the device
	if (ioctl(fd, EVIOCSFF, &gamepad->ctx->effect) == -1) {
		perror("could not set rumble");
		return;
	}

	// construct a "play" input event and write that to the device
	struct input_event play = {0};
	play.type = EV_FF;
	play.code = (uint16_t)gamepad->ctx->effect.id;
	play.value = 1;
	if (write(fd, (const void*)&play, sizeof(play)) == -1) {
		perror("error writing rumble packet");
		return;
	}

	// Note that we don't erase the event after uploading it, because this
	// causes the effect to cancel before it even starts.
}
