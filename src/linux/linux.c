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

  struct mg_gamepad_t *gamepads_list = NULL;
  size_t gamepad_len = 0;

  struct mg_gamepads_t *gamepads = malloc(sizeof(struct mg_gamepads_t));

  // for each file found:
  while ((dp = readdir(dfd)) != NULL) {
    // get the full path of it
    char full_path[256];
    snprintf(full_path, 256, "/dev/input/by-id/%s", dp->d_name);

    // open said full path with libevdev
    struct libevdev *dev = libevdev_new();
    if (libevdev_set_fd(dev, open(full_path, O_RDONLY))) {
      // char err[256];
      // snprintf(err, 256, "could not open %s", full_path);
      // perror(err);
      continue;
    };

    bool good = false;
    mg_gamepad_btn_map_type buttons[MAX_BUTTONS];
    mg_gamepad_axis_map_type axises[MAX_AXISES];
    mg_gamepad_axis_map_type deadzones[MAX_AXISES];
    size_t button_len = 0;
    size_t axis_len = 0;
    size_t deadzones_len = 0;
    struct mg_gamepad_t gamepad;
    // go through any buttons a gamepad would have
    for (int i = BTN_MISC; i <= BTN_TRIGGER_HAPPY6; i++) {
      // if this device has one...
      if (libevdev_has_event_code(dev, EV_KEY, i)) {
        // On the first run, we're gonna save this device, and signify to the
        // below code that we have something.
        if (!good) {
          gamepad.dev = dev;
          good = true;
        }
        // and put the gamepad button we have down.
        gamepad.buttons[button_len] = (mg_gamepad_btn_map_type){
            .key = get_gamepad_btn(i),
            .value = 0,
        };
        button_len += 1;
      }
    }
    // go through any axises a gamepad would have
    for (int i = ABS_X; i <= ABS_MAX; i++) {
      if (libevdev_has_event_code(dev, EV_ABS, i)) {
        if (!good) {
          gamepad.dev = dev;
          good = true;
        }

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

        gamepad.axises[axis_len] =
            (mg_gamepad_axis_map_type){.key = get_gamepad_axis(i), .value = 0};

        gamepad.deadzones[axis_len] = (mg_gamepad_axis_map_type){
            .key = get_gamepad_axis(i), .value = deadzone};

        axis_len += 1;
      }
    }

    if (good) {
      gamepad.button_len = button_len;
      gamepad.axis_len = axis_len;
      gamepads->gamepads_list[gamepad_len] = gamepad;
      gamepad_len += 1;
    } else {
      if (dev != NULL) {
        libevdev_free(dev);
      }
    }
  }

  gamepads->gamepads_list_len = gamepad_len;

  return gamepads;
};

size_t mg_gamepads_num(mg_gamepads *gamepads) {
  return gamepads->gamepads_list_len;
};
mg_gamepad *mg_gamepads_at(mg_gamepads *gamepads, size_t idx) {
  return &gamepads->gamepads_list[idx];
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

    for (int i = 0; i <= gamepad->button_len; i++) {
      if (gamepad->buttons[i].key == btn) {
        gamepad->buttons[i].value = ev.value;
      }
    }
    break;
  }
  case EV_ABS: {
    mg_gamepad_axis axis = get_gamepad_axis(ev.code);

    for (int i = 0; i <= gamepad->axis_len; i++) {
      if (gamepad->axises[i].key == axis) {
        int deadzone = gamepad->deadzones[i].value;
        int event_val = 0;
        if (abs(ev.value) >= deadzone) {
          event_val = ev.value;
        }
        mg_gamepad_axis_map_type value = {
            .key = axis,
            .value = event_val,
        };
        gamepad->axises[i] = value;
      }
    }
    break;
  }
  default:
    break;
  }
}

int mg_gamepad_get_button_status(mg_gamepad *gamepad, mg_gamepad_btn btn) {
  for (int i = 0; i < gamepad->button_len; i++) {
    if (gamepad->buttons[i].key == btn) {
      return gamepad->buttons[i].value;
    }
  }
  return -1;
}

size_t mg_gamepad_btns_num(mg_gamepad *gamepad) { return gamepad->button_len; };
mg_gamepad_btn mg_gamepad_btns_at(mg_gamepad *gamepad, size_t idx) {
  return gamepad->buttons[idx].key;
};

size_t mg_gamepad_get_axis_num(mg_gamepad *gamepad) {
  return gamepad->axis_len;
}

int mg_gamepad_get_axis_status(mg_gamepad *gamepad, size_t axis) {
  for (int i = 0; i < gamepad->axis_len; i++) {
    if (gamepad->axises[i].key == axis) {
      return gamepad->axises[i].value;
    }
  }
  return -1;
}

mg_gamepad_axis mg_gamepad_axis_at(mg_gamepad *gamepad, size_t idx) {
  return gamepad->axises[idx].key;
}

size_t mg_gamepad_get_axis_deadzone(mg_gamepad *gamepad, size_t axis) {
  for (int i = 0; i < gamepad->axis_len; i++) {
    if (gamepad->deadzones[i].key == axis) {
      return gamepad->deadzones[i].value;
    }
  }
  return -1;
};
void mg_gamepad_set_axis_deadzone(mg_gamepad *gamepad, size_t axis,
                                  size_t deadzone) {
  for (int i = 0; i < gamepad->axis_len; i++) {
    if (gamepad->deadzones[i].key == axis) {
      gamepad->deadzones[i].value = deadzone;
    }
  }
};
