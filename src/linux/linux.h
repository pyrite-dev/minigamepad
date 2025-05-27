#include "libevdev/libevdev.h"
#include "minigamepad.h"
#include <stdint.h>

#ifndef __minigamepad_LINUX
#define __minigamepad_LINUX

struct mg_gamepad_context_t {
  struct libevdev *dev;
  struct input_event input_event;
};

mg_gamepad_btn get_gamepad_btn(int btn);
mg_gamepad_axis get_gamepad_axis(int axis);
int get_native_btn(mg_gamepad_btn btn);
int get_native_axis(mg_gamepad_axis axis);

#endif