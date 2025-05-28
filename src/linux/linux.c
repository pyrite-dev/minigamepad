#include "linux.h"
#include "libevdev/libevdev.h"
#include "minigamepad.h"
#include <dirent.h>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

mg_gamepads *mg_gamepads_get(void) {
  struct dirent *dp;
  DIR *dfd;

  // open the directory where all the devices are gonna be
  if ((dfd = opendir("/dev/input/by-id")) == NULL) {
    fprintf(stderr, "Can't open /dev/input/by-id/\n");
    return 0;
  }

  size_t gamepad_num = 0;

  struct mg_gamepads_t *gamepads = malloc(sizeof(struct mg_gamepads_t));

  // for each file found:
  while ((dp = readdir(dfd)) != NULL) {
    // get the full path of it
    char full_path[300];
    snprintf(full_path, sizeof(full_path), "/dev/input/by-id/%s", dp->d_name);

    // open said full path with libevdev
    struct libevdev *dev = libevdev_new();
    if (libevdev_set_fd(dev, open(full_path, O_RDWR))) {
      // char err[256];
      // snprintf(err, 256, "could not open %s", full_path);
      // perror(err);
      continue;
    };

    bool good = false;
    size_t button_num = 0;
    size_t axis_num = 0;
    struct mg_gamepad_t gamepad = {0};
    gamepad.ctx = malloc(sizeof(struct mg_gamepad_context_t));

    // go through any buttons a gamepad would have
    for (unsigned int i = BTN_MISC; i <= BTN_TRIGGER_HAPPY6; i++) {
      // if this device has one...
      if (libevdev_has_event_code(dev, EV_KEY, i)) {
        // On the first run, we're gonna save this device, and signify to the
        // below code that we have something.
        if (!good) {
          gamepad.ctx->dev = dev;
          good = true;
        }
        // and put the gamepad button we have down.
        gamepad.buttons[button_num].key = get_gamepad_btn(i);
        gamepad.buttons[button_num].value = 0;
        button_num += 1;
      }
    }
    // go through any axises a gamepad would have
    for (unsigned int i = ABS_X; i <= ABS_MAX; i++) {
      if (libevdev_has_event_code(dev, EV_ABS, i)) {
        if (!good) {
          gamepad.ctx->dev = dev;
          good = true;
        }

        // We want every axis except the hats (which are usually d-pads)
        // to have a deadzone of 5000. The idea is that programmer can override
        // this later, by a config file or something.
        int16_t deadzone = 0;
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

        gamepad.axises[axis_num].key = get_gamepad_axis(i);
        gamepad.axises[axis_num].value = 0;
        gamepad.axises[axis_num].deadzone = deadzone;
        axis_num += 1;
      }
    }

    if (good) {

      if (libevdev_has_event_code(
              dev, EV_FF,
              FF_RUMBLE)) { // this is a struct that gets passed to the rumble
                            // effect when
        // activated. we have to "erase" the effect whenever we want to add a
        // new one, hence this gets put in the struct itself so that we can keep
        // track of the id. and while we're here we save some other values
        // that'll never get changed.
        gamepad.ctx->effect = (struct ff_effect){
            .type = FF_RUMBLE,
            .id = -1,
            .direction = 0,
            .trigger = {0, 0},
            .replay =
                {
                    .delay = 0,
                },
        };
        gamepad.ctx->supports_rumble = true;
      } else {
        gamepad.ctx->supports_rumble = false;
      }
      gamepad.button_num = button_num;
      gamepad.axis_num = axis_num;

      gamepads->list[gamepad_num] = malloc(sizeof(gamepad));
      *gamepads->list[gamepad_num] = gamepad;

      gamepad_num += 1;
    } else {
      if (dev != NULL) {
        libevdev_free(dev);
      }
    }
  }

  gamepads->num = gamepad_num;

  return gamepads;
}

size_t mg_gamepads_num(mg_gamepads *gamepads) { return gamepads->num; }

void mg_gamepads_free(mg_gamepads *gamepads) { free(gamepads); }

const char *mg_gamepad_get_name(mg_gamepad *gamepad) {
  return libevdev_get_name(gamepad->ctx->dev);
}

void mg_gamepad_update(mg_gamepad *gamepad) {
  struct input_event ev;
  int pending = libevdev_has_event_pending(gamepad->ctx->dev);
  if (pending) {
    int rc = libevdev_next_event(gamepad->ctx->dev, LIBEVDEV_READ_FLAG_BLOCKING,
                                 &ev);
    if (rc) {
      return;
    }
  } else {
    return;
  }

  switch (ev.type) {
  case EV_KEY: {
    mg_gamepad_btn btn = get_gamepad_btn(ev.code);

    for (size_t i = 0; i <= gamepad->button_num; i++) {
      if (gamepad->buttons[i].key == btn) {
        gamepad->buttons[i].value = (int16_t)ev.value;
      }
    }
    break;
  }
  case EV_ABS: {
    mg_gamepad_axis axis = get_gamepad_axis(ev.code);

    for (unsigned int i = 0; i <= gamepad->axis_num; i++) {
      if (gamepad->axises[i].key == axis) {
        int deadzone = gamepad->axises[i].deadzone;
        int16_t event_val = 0;
        if (abs(ev.value) >= deadzone) {
          event_val = (int16_t)ev.value;
        }
        gamepad->axises[i].key = axis;
        gamepad->axises[i].value = event_val;
      }
    }
    break;
  }
  case EV_FF: {
    printf("EV_FF: %d\n", ev.value);
  }
  default:
    break;
  }
}

int mg_gamepad_get_button_status(mg_gamepad *gamepad, mg_gamepad_btn btn) {
  for (unsigned int i = 0; i < gamepad->button_num; i++) {
    if (gamepad->buttons[i].key == btn) {
      return gamepad->buttons[i].value;
    }
  }
  return -1;
}

size_t mg_gamepad_btns_num(mg_gamepad *gamepad) { return gamepad->button_num; }
mg_gamepad_btn mg_gamepad_btns_at(mg_gamepad *gamepad, size_t idx) {
  return gamepad->buttons[idx].key;
}

size_t mg_gamepad_get_axis_num(mg_gamepad *gamepad) {
  return gamepad->axis_num;
}

int mg_gamepad_get_axis_status(mg_gamepad *gamepad, size_t axis) {
  for (unsigned int i = 0; i < gamepad->axis_num; i++) {
    if (gamepad->axises[i].key == axis) {
      return gamepad->axises[i].value;
    }
  }
  return -1;
}

mg_gamepad_axis mg_gamepad_axis_at(mg_gamepad *gamepad, size_t idx) {
  return gamepad->axises[idx].key;
}

void mg_gamepad_rumble(mg_gamepad *gamepad, uint16_t strong_vibration,
                       uint16_t weak_vibration, uint16_t milliseconds) {
  // only continue if the controller does rumble
  if (!gamepad->ctx->supports_rumble) {
    return;
  }

  // libevdev doesn't support rumble so we have to do it raw.

  // get the fd that libevdev is holding for the input device
  int fd = libevdev_get_fd(gamepad->ctx->dev);

  //  if we currently have an effect going on, erase it to make room for the new
  //  one.
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
  play.code = gamepad->ctx->effect.id;
  play.value = 1;
  if (write(fd, (const void *)&play, sizeof(play)) == -1) {
    perror("error writing rumble packet");
    return;
  }

  // Note that we don't erase the event after uploading it, because this causes
  // the effect to cancel before it even starts.
}