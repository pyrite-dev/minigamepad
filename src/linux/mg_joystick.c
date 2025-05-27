#include "libevdev/libevdev.h"
#include "linux.h"
#include "minigamepad.h"
#include <linux/input.h>
#include <stdio.h>

const char *mg_gamepad_get_name(mg_gamepad *gamepad) {
  return libevdev_get_name(gamepad->dev);
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
  if (ev.type != EV_KEY) {
    return;
  }
  mg_gamepad_btn btn = get_gamepad_btn(ev.code);

  if (hmgeti(gamepad->buttons, btn) != -1) {
    mg_gamepad_btn_map_type value = {
        .key = btn,
        .value = ev.value,
    };
    hmputs(gamepad->buttons, value);
  }
}