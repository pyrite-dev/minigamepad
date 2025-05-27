#include "winmm.h"
#include "minigamepad.h"
#include <stdlib.h>

mg_gamepads *mg_gamepads_get() {
  return NULL;
};

size_t mg_gamepads_num(mg_gamepads *gamepads) {
  return 0;
};
mg_gamepad *mg_gamepads_at(mg_gamepads *gamepads, size_t idx) {
  return NULL;
};

void mg_gamepads_free(mg_gamepads *gamepads) {  };

const char *mg_gamepad_get_name(mg_gamepad *gamepad) {
  return NULL;
}

void mg_gamepad_update(mg_gamepad *gamepad) {
}

int mg_gamepad_get_button_status(mg_gamepad *gamepad, mg_gamepad_btn btn) {
  return -1;
}

size_t mg_gamepad_btns_num(mg_gamepad *gamepad) { return -1; };
mg_gamepad_btn mg_gamepad_btns_at(mg_gamepad *gamepad, size_t idx) {
  return 0;
};

size_t mg_gamepad_get_axis_num(mg_gamepad *gamepad) {
  return 0;
}

int mg_gamepad_get_axis_status(mg_gamepad *gamepad, size_t axis) {
  return 0;
}

mg_gamepad_axis mg_gamepad_axis_at(mg_gamepad *gamepad, size_t idx) {
  return 0;
}

size_t mg_gamepad_get_axis_deadzone(mg_gamepad *gamepad, size_t axis) {
  return 0;
};
void mg_gamepad_set_axis_deadzone(mg_gamepad *gamepad, size_t axis,
                                  size_t deadzone) {
  return;
};
