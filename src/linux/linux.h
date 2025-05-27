#include "../stb/stb_ds.h"
#include "libevdev/libevdev.h"
#include "minigamepad.h"

#ifndef __minigamepad_LINUX
#define __minigamepad_LINUX

typedef struct {
  mg_gamepad_btn key;
  int value;
} mg_gamepad_btn_map_type;

struct mg_gamepad_t {
  struct libevdev *dev;
  mg_gamepad_btn_map_type *buttons;
  size_t button_len;
};
struct mg_gamepads_t {
  struct mg_gamepad_t *joysticks_list;
  size_t joysticks_list_len;
};

mg_gamepad_btn get_gamepad_btn(int btn);
int get_native_btn(mg_gamepad_btn btn);

#endif