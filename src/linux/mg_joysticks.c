#include "linux.h"
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
    char *full_path = malloc(255);
    snprintf(full_path, 255, "/dev/input/by-id/%s", dp->d_name);

    // open said full path with libevdev
    struct libevdev *dev = libevdev_new();
    if (libevdev_set_fd(dev, open(full_path, O_RDONLY))) {
      // char *err = malloc(255);
      // snprintf(err, 255, "could not open %s", full_path);
      // perror(err);
      // free(err);
      continue;
    };

    bool good = false;
    mg_gamepad_btn_map_type *gamepad_buttons = NULL;
    struct mg_gamepad_t joystick;
    // go through any button a joystick would have
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
    if (good) {
      joystick.buttons = gamepad_buttons;
      joystick.button_len = arrlen(gamepad_buttons);

      arrpush(joysticks_list, joystick);
    } else {
      if (dev != NULL) {
        libevdev_free(dev);
      }
    }

    free(full_path);
  }

  struct mg_gamepads_t *joysticks = malloc(sizeof(struct mg_gamepads_t));
  joysticks->joysticks_list = joysticks_list;
  joysticks->joysticks_list_len = arrlen(joysticks_list);

  return joysticks;
};

size_t mg_gamepads_num(mg_gamepads *mj) { return mj->joysticks_list_len; };
mg_gamepad *mg_gamepads_at(mg_gamepads *mj, size_t idx) {
  return &mj->joysticks_list[idx];
};

void mg_gamepads_free(mg_gamepads *mj) { free(mj); };
