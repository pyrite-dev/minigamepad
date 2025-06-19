#include "libevdev/libevdev.h"
#include "minigamepad.h"
#include <stdint.h>

#ifndef __minigamepad_LINUX
#define __minigamepad_LINUX

struct mg_gamepad_context_t {
  struct libevdev *dev;           // libevdev context
  struct input_event input_event; // input event used in the libevdev function
  uint8_t supports_rumble;        // whether this controller does rumble
  struct ff_effect effect;        // effect used in the rumble function
  char full_path[267];            // the path of the file
  unsigned int keyMap[KEY_CNT - BTN_MISC];
  unsigned int absMap[ABS_CNT];
  struct input_absinfo absInfo[ABS_CNT];
};

mg_gamepad_btn get_gamepad_btn(unsigned int btn);
mg_gamepad_axis get_gamepad_axis(unsigned int axis);
int get_native_btn(mg_gamepad_btn btn);
int get_native_axis(mg_gamepad_axis axis);

bool setup_gamepad(mg_gamepads *gamepads, char *full_path);
#endif
