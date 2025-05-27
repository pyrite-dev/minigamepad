#include "linux.h"
#include "libevdev/libevdev.h"
#include "minigamepad.h"
#include <dirent.h>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

mg_gamepads *mg_gamepads_get() {
  struct dirent *dp;
  DIR *dfd;

  // open the directory where all the devices are gonna be
  if ((dfd = opendir("/dev/input/by-id")) == NULL) {
    fprintf(stderr, "Can't open /dev/input/by-id/\n");
    return 0;
  }

  struct mg_gamepad_t *joysticks_list = NULL;

  // for each file found:
  while ((dp = readdir(dfd)) != NULL) {
    // get the full path of it
    char full_path[256];
    snprintf(full_path, 256, "/dev/input/by-id/%s", dp->d_name);

    // open said full path with libevdev
    struct libevdev *dev = libevdev_new();
    if (libevdev_set_fd(dev, open(full_path, O_RDONLY))) {
      // char err[256]
      // snprintf(err, 256, "could not open %s", full_path);
      // perror(err);
      continue;
    };

    bool good = false;
    mg_gamepad_btn_map_type *gamepad_buttons = NULL;
    mg_gamepad_axis_map_type *axises = NULL;
    mg_gamepad_axis_map_type *deadzones = NULL;
    struct mg_gamepad_t joystick;
    // go through any buttons a joystick would have
    for (int i = BTN_MISC; i <= BTN_GEAR_UP; i++) {
      // if this device has one...
      if (libevdev_has_event_code(dev, EV_KEY, i)) {
        // On the first run, we're gonna save this device, and signify to the
        // below code that we have something.
        if (!good) {
          joystick.dev = dev;
          good = true;
        }
        // and put the gamepad button we have down.
        hmput(gamepad_buttons, get_gamepad_btn(i), 0);
      }
    }
    // go through any axises a joystick would have
    for (int i = ABS_X; i <= ABS_MAX; i++) {
      if (libevdev_has_event_code(dev, EV_ABS, i)) {
        if (!good) {
          joystick.dev = dev;
          good = true;
        }
        hmput(axises, get_gamepad_axis(i), 0);

        // We want every axis except the hats (which are usually d-pads)
        // to have a deadzone of 5000. The idea is that programmer can override
        // this later, by a config file or something.
        int deadzone = 0;
        switch (i) {
        case ABS_HAT0X:
        case ABS_HAT0Y:
        case ABS_HAT1X:
        case ABS_HAT1Y:
        case ABS_HAT2X:
        case ABS_HAT2Y:
        case ABS_HAT3X:
        case ABS_HAT3Y:
          break;
        default:
          deadzone = 5000;
          break;
        }
        hmput(deadzones, get_gamepad_axis(i), deadzone);
      }
    }

    if (good) {
      joystick.buttons = gamepad_buttons;
      joystick.button_len = arrlen(gamepad_buttons);
      joystick.axises = axises;
      joystick.axis_len = arrlen(axises);
      joystick.deadzones = deadzones;
      arrpush(joysticks_list, joystick);
    } else {
      if (dev != NULL) {
        libevdev_free(dev);
      }
    }
  }

  struct mg_gamepads_t *joysticks = malloc(sizeof(struct mg_gamepads_t));
  joysticks->joysticks_list = joysticks_list;
  joysticks->joysticks_list_len = arrlen(joysticks_list);

  return joysticks;
};

size_t mg_gamepads_num(mg_gamepads *gamepads) {
  return gamepads->joysticks_list_len;
};
mg_gamepad *mg_gamepads_at(mg_gamepads *gamepads, size_t idx) {
  return &gamepads->joysticks_list[idx];
};

void mg_gamepads_free(mg_gamepads *gamepads) { free(gamepads); };

const char *mg_gamepad_get_name(mg_gamepad *gamepad) {
  return libevdev_get_name(gamepad->dev);
}

void mg_gamepad_update(mg_gamepad *gamepad) {
  struct input_event ev;
  int pending = libevdev_has_event_pending(gamepad->dev);
  if (pending) {
    int rc =
        libevdev_next_event(gamepad->dev, LIBEVDEV_READ_FLAG_BLOCKING, &ev);
    if (rc) {
      return;
    }
  } else {
    return;
  }

  // // take this opprutunity to reset every axis
  // for (int i = 0; i < gamepad->axis_len; i++) {
  //   gamepad->axises[i].value = 0;
  // }

  switch (ev.type) {
  case EV_KEY: {
    mg_gamepad_btn btn = get_gamepad_btn(ev.code);

    if (hmgeti(gamepad->buttons, btn) != -1) {
      mg_gamepad_btn_map_type value = {
          .key = btn,
          .value = ev.value,
      };
      hmputs(gamepad->buttons, value);
    }
    break;
  }
  case EV_ABS: {
    mg_gamepad_axis axis = get_gamepad_axis(ev.code);

    if (hmgeti(gamepad->axises, axis) != -1) {
      int deadzone = hmget(gamepad->deadzones, axis);
      int event_val = 0;
      if (abs(ev.value) >= deadzone) {
        event_val = ev.value;
      } else {
        event_val = 0;
      }
      mg_gamepad_axis_map_type value = {
          .key = axis,
          .value = event_val,
      };
      hmputs(gamepad->axises, value);
    }
    break;
  }
  default:
    break;
  }
}

int mg_gamepad_get_button_status(mg_gamepad *gamepad, mg_gamepad_btn btn) {
  int native_btn = get_native_btn(btn);
  if (hmgeti(gamepad->buttons, btn) != -1) {
    return hmget(gamepad->buttons, btn);
  } else {
    return -1;
  }
}

size_t mg_gamepad_btns_num(mg_gamepad *gamepad) { return gamepad->button_len; };
mg_gamepad_btn mg_gamepad_btns_at(mg_gamepad *gamepad, size_t idx) {
  return gamepad->buttons[idx].key;
};

size_t mg_gamepad_get_axis_num(mg_gamepad *gamepad) {
  return gamepad->axis_len;
}

int mg_gamepad_get_axis_status(mg_gamepad *gamepad, size_t axis) {
  int native_axis = get_native_axis(axis);
  if (hmgeti(gamepad->axises, axis) != -1) {
    return hmget(gamepad->axises, axis);
  } else {
    return 0;
  }
}

mg_gamepad_axis mg_gamepad_axis_at(mg_gamepad *gamepad, size_t idx) {
  return gamepad->axises[idx].key;
}

size_t mg_gamepad_get_axis_deadzone(mg_gamepad *gamepad, size_t axis) {
  int native_axis = get_native_axis(axis);
  if (hmgeti(gamepad->deadzones, axis) != -1) {
    return hmget(gamepad->deadzones, axis);
  } else {
    return 0;
  }
};
void mg_gamepad_set_axis_deadzone(mg_gamepad *gamepad, size_t axis,
                                  size_t deadzone) {

  if (hmgeti(gamepad->deadzones, axis) != -1) {
    mg_gamepad_axis_map_type value = {
        .key = axis,
        .value = deadzone,
    };
    hmputs(gamepad->axises, value);
  }
};
