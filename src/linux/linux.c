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

            global.watch = inotify_add_watch(global.inotify, "/dev/input/", IN_CREATE | IN_ATTRIB | IN_DELETE);
        }
    }

    struct dirent *dp;
    DIR *dfd;

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

    struct mg_gamepad_context_t* ctx = malloc(sizeof(struct mg_gamepad_context_t));

    gamepad->ctx = ctx;
    gamepad->button_num = 0;

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
        ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absBits)), absBits) < 0
    ) {
        mg_gamepad_remove(gamepads, gamepad);
        return false;
    }

    #define isBitSet(bit, arr) (arr[(bit) / 8] & (1 << ((bit) % 8)))
    if (!isBitSet(EV_ABS, evBits)) {
        mg_gamepad_remove(gamepads, gamepad);
        return false;
    }

    // go through any buttons a gamepad would have
    for (unsigned int i = BTN_MISC; i < KEY_CNT; i++) {
        if (!isBitSet(i, keyBits))
            continue;

        gamepad->ctx->keyMap[i - BTN_MISC] = (int)gamepad->button_num;
        gamepad->button_num++;
    }

    #undef isBitSet
    // go through any axises a gamepad would have
    for (unsigned int i = ABS_X; i <= ABS_MAX; i++) {
        if (libevdev_has_event_code(ctx->dev, EV_ABS, i)) {
            gamepad->axis_num += 1;
        }
    }


    if ((gamepad->button_num == 0 && gamepad->axis_num == 0) || gamepad->button_num > MG_GAMEPAD_BUTTON_MAX + 10) {
        mg_gamepad_remove(gamepads, gamepad);
        return false;
    }

    memcpy(gamepad->ctx->full_path, full_path, sizeof(gamepad->ctx->full_path));            
    // Generate a joystick GUID that matches the SDL 2.0.5+ one (sourced from GLFW)
    const char* name = libevdev_get_name(gamepad->ctx->dev);
    if (id.vendor && id.product && id.version) {
        snprintf(gamepad->guid, sizeof(gamepad->guid), "%02x%02x0000%02x%02x0000%02x%02x0000%02x%02x0000",
                 id.bustype & 0xff, id.bustype >> 8,
                 id.vendor & 0xff,  id.vendor >> 8,
                 id.product & 0xff, id.product >> 8,
                 id.version & 0xff, id.version >> 8);
    } else {
        snprintf(gamepad->guid, sizeof(gamepad->guid), "%02x%02x0000%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x00",
                 id.bustype & 0xff, id.bustype >> 8,
                 name[0], name[1], name[2], name[3],
                 name[4], name[5], name[6], name[7],
                 name[8], name[9], name[10]);
    } 

    strncpy(gamepad->name, name, sizeof(gamepad->name) - 1);
    gamepad->mapping = mg_gamepad_find_valid_mapping(gamepad);

    unsigned int i = 0;
    for (unsigned int btn = BTN_MISC; btn < KEY_CNT; btn++) {
        if (libevdev_has_event_code(ctx->dev, EV_KEY, btn) == false) {
            continue;
        }

         gamepad->buttons[i].key = mg_get_gamepad_btn(gamepad, (unsigned int) gamepad->ctx->keyMap[btn - BTN_MISC]); 
        gamepad->buttons[i].value = 0;
        i += 1;
    }

    i = 0;
    for (unsigned int axis = ABS_X; axis <= ABS_MAX; axis++) {
        if (libevdev_has_event_code(ctx->dev, EV_ABS, i) == false) {
            continue;
        }
 
        // We want every axis except the hats (which are usually d-pads)
        // to have a deadzone of 5000. The idea is that programmer can override
        // this later, by a config file or something.
        int16_t deadzone = 0;
        switch (axis) {
            case ABS_HAT0X:
            case ABS_HAT0Y:
            case ABS_HAT1X:
            case ABS_HAT1Y:
            case ABS_HAT2X:
            case ABS_HAT2Y:
            case ABS_HAT3X:
            case ABS_HAT3Y:
                gamepad->hat_num += 1;
                break;
            default:
                deadzone = 5000;
                break;
        }

        gamepad->axises[i].key = mg_get_gamepad_axis(gamepad, axis);
        gamepad->axises[i].value = 0;
        gamepad->axises[i].deadzone = deadzone;
        i += 1;
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

bool mg_gamepads_fetch(mg_gamepads *gamepads) {
    if (global.inotify <= 0)
        return false;

    ssize_t offset = 0;
    char buffer[16384];
    const ssize_t size = read(global.inotify, buffer, sizeof(buffer));

    char full_path[273];
    const char path[] = "/dev/input/";

    bool ret = false;
    while (size > offset) {
        const struct inotify_event* e = (struct inotify_event*) (buffer + offset);

        offset += (ssize_t)sizeof(struct inotify_event) + e->len;

        char* substr = strstr(e->name, "-joystick");
        size_t len = sizeof(path);
        
        if (substr) {
            len += (size_t)(substr - e->name) + 7;
        } else {
            continue;
        }

        snprintf(full_path, len, "%s%s", path, &e->name[2]);
        full_path[len] = '\0';

        if (e->mask & (IN_CREATE | IN_ATTRIB)) { 
            if (setup_gamepad(gamepads, full_path)) {
                ret = true;
            }
        }
    
        else if (e->mask & IN_DELETE) {   
            for (mg_gamepad* cur = gamepads->head; cur != NULL; cur = cur->next) {
                if (strncmp(cur->ctx->full_path, full_path, sizeof(cur->ctx->full_path)) == 0) {
                    mg_gamepad_remove(gamepads, cur);
                    break;
                }
            }
        }
    }

    return ret;
}

void mg_gamepad_free(mg_gamepad *gamepad) { 
    libevdev_free(gamepad->ctx->dev);
    free(gamepad->ctx);
}

bool mg_gamepad_update(mg_gamepad *gamepad, mg_gamepad_event* event) {    
    if (gamepad->connected == false) return false;
    // Check if the file we opened the gamepad at still exists.
    // Fun fact, this was done after an hour of trying other methods, including
    // installing an inotify watcher and seeing if libevdev reports this. This is
    // appearently the best way.
    bool can_access = access(gamepad->ctx->full_path, F_OK) == 0;

    if (can_access == false) {
        if (event != NULL) {
            event->gamepad = gamepad;
            event->type = MG_GAMEPAD_DISCONNECT; 
        }

        gamepad->connected = false;
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

            mg_gamepad_btn btn = mg_get_gamepad_btn(gamepad, (unsigned int) gamepad->ctx->keyMap[ev.code - BTN_MISC]); 

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
            mg_gamepad_axis axis = mg_get_gamepad_axis(gamepad, ev.code);

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
