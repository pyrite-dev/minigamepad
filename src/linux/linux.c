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

#define MG_UNUSED(x) (void)(x)
void mg_gamepads_backend_init(mg_gamepads* gamepads) {
    MG_UNUSED(gamepads);
    // linux does not need an init function
}

bool mg_gamepads_fetch(mg_gamepads *gamepads) {
    struct dirent *dp;
    DIR *dfd;

    // open the directory where all the devices are gonna be
    if ((dfd = opendir("/dev/input/by-id")) == NULL) {
        fprintf(stderr, "Can't open /dev/input/by-id/\n");
        return false;
    }

    bool ret = false;

    // for each file found:
    while ((dp = readdir(dfd)) != NULL) {
        // get the full path of it
        char full_path[300];
        snprintf(full_path, sizeof(full_path), "/dev/input/by-id/%s", dp->d_name);

        // TODO: there has to be a better way to do this 
        bool found = false;
        for (struct mg_gamepad_t* cur = gamepads->head; cur != NULL; cur = cur->next) {
            if (strncmp(cur->ctx->full_path, full_path, sizeof(full_path)) == 0) {
                found = true;
                break;
            }
        }

        if (found) {
            continue;
        }

        struct mg_gamepad_t* gamepad = mg_alloc(gamepads);
        if (gamepad == NULL) {
            break; 
        }
        
        struct mg_gamepad_context_t* ctx = malloc(sizeof(struct mg_gamepad_context_t));

        gamepad->ctx = ctx;

        // open said full path with libevdev
        ctx->dev = libevdev_new();
        if (libevdev_set_fd(ctx->dev, open(full_path, O_RDWR))) {
            // char err[256];
            // snprintf(err, 256, "could not open %s", full_path);
            // perror(err);
            libevdev_free(ctx->dev);
            continue;
        };

        size_t button_num = 0;
        size_t axis_num = 0;

        // go through any buttons a gamepad would have
        for (unsigned int i = BTN_MISC; i <= BTN_TRIGGER_HAPPY6; i++) {
            // if this device has one...
            if (libevdev_has_event_code(ctx->dev, EV_KEY, i)) {
                gamepad->buttons[button_num].key = get_gamepad_btn(i);
                gamepad->buttons[button_num].value = 0;
                button_num += 1;
            }
        }
        // go through any axises a gamepad would have
        for (unsigned int i = ABS_X; i <= ABS_MAX; i++) {
            if (libevdev_has_event_code(ctx->dev, EV_ABS, i)) {
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

                gamepad->axises[axis_num].key = get_gamepad_axis(i);
                gamepad->axises[axis_num].value = 0;
                gamepad->axises[axis_num].deadzone = deadzone;
                axis_num += 1;
            }
        }

        if (button_num || axis_num) {
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
            gamepad->button_num = button_num;
            gamepad->axis_num = axis_num;
            memcpy(gamepad->ctx->full_path, full_path, sizeof(full_path));
            ret = true;
        } else {
            mg_gamepad_remove(gamepads, gamepad);
        }
    }

    closedir(dfd);

    return ret;
}

void mg_gamepad_free(mg_gamepad *gamepad) { 
    libevdev_free(gamepad->ctx->dev);
    free(gamepad->ctx);
}

const char *mg_gamepad_get_name(mg_gamepad *gamepad) {
    return libevdev_get_name(gamepad->ctx->dev);
}

#define INOTIFY_BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

bool mg_gamepad_update(mg_gamepad *gamepad, mg_gamepad_event* event) {
    if (mg_gamepad_is_connected(gamepad) == false) {
        if (event != NULL) {
            event->gamepad = gamepad;
            event->type = MG_GAMEPAD_DISCONNECT; 
        }
        return true;
    }


    // go through libevdev events.
    struct input_event ev;
    int pending = libevdev_has_event_pending(gamepad->ctx->dev);
    if (pending) {
        int rc = libevdev_next_event(gamepad->ctx->dev, LIBEVDEV_READ_FLAG_BLOCKING,
                                     &ev);
        if (rc) {
            return false;
        }
    } else {
        return false;
    }

    switch (ev.type) {
        case EV_KEY: {
            mg_gamepad_btn btn = get_gamepad_btn(ev.code);

            for (size_t i = 0; i <= gamepad->button_num; i++) {
                if (gamepad->buttons[i].key == btn) {
                    gamepad->buttons[i].value = (int16_t)ev.value;
                }
            }
           
            if (event != NULL) {
                event->gamepad = gamepad;
                event->btn = btn;
                event->type = ev.value ? MG_GAMEPAD_BTN_PRESS  : MG_GAMEPAD_BTN_RELEASE; 
            }
            return true;
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
           
            if (event != NULL) {
                event->type = MG_GAMEPAD_AXIS_MOVE;
                event->gamepad = gamepad;
                event->axis = axis;
            }
            return true;
        }
        case EV_FF: {
        }
        default:
            break;
    }

    return false;
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
  play.code = (uint16_t)gamepad->ctx->effect.id;
  play.value = 1;
  if (write(fd, (const void *)&play, sizeof(play)) == -1) {
    perror("error writing rumble packet");
    return;
  }

  // Note that we don't erase the event after uploading it, because this causes
  // the effect to cancel before it even starts.
}

bool mg_gamepad_is_connected(mg_gamepad *gamepad) {
  // Check if the file we opened the gamepad at still exists.
  // Fun fact, this was done after an hour of trying other methods, including
  // installing an inotify watcher and seeing if libevdev reports this. This is
  // appearently the best way.
  return access(gamepad->ctx->full_path, F_OK) == 0;
}
