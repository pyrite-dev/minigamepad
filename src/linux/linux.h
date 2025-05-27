#include "libevdev/libevdev.h"
#include "minigamepad.h"
#include <stdint.h>

#ifndef __minigamepad_LINUX
#define __minigamepad_LINUX

typedef struct {
  mg_gamepad_btn key;
  int16_t value;
} mg_gamepad_btn_map_type;

typedef struct {
  mg_gamepad_axis key;
  int16_t value;
} mg_gamepad_axis_map_type;

struct mg_gamepad_t {
  struct libevdev *dev;
  mg_gamepad_btn_map_type buttons[MAX_BUTTONS];
  mg_gamepad_axis_map_type axises[MAX_AXISES];
  mg_gamepad_axis_map_type deadzones[MAX_AXISES];
  size_t button_len;
  size_t axis_len;
  // deadzone_len is the same as axis_len
};
struct mg_gamepads_t {
  struct mg_gamepad_t gamepads_list[16];
  size_t gamepads_list_len;
};

mg_gamepad_btn get_gamepad_btn(int btn);
mg_gamepad_axis get_gamepad_axis(int axis);
int get_native_btn(mg_gamepad_btn btn);
int get_native_axis(mg_gamepad_axis axis);

#endif