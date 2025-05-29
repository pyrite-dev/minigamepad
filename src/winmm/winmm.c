#include "winmm.h"
#include "minigamepad.h"
#include <stdlib.h>

#define MG_UNUSED(x) (void)(x)

mg_gamepads *mg_gamepads_fetch(void) { return NULL; }

size_t mg_gamepads_num(mg_gamepads *gamepads) {
  MG_UNUSED(gamepads);
  return 0;
}
mg_gamepad *mg_gamepads_at(mg_gamepads *gamepads, size_t idx) {
  MG_UNUSED(gamepads);
  MG_UNUSED(idx);
  return NULL;
}

void mg_gamepads_free(mg_gamepads *gamepads) { MG_UNUSED(gamepads); }

const char *mg_gamepad_get_name(mg_gamepad *gamepad) {
  MG_UNUSED(gamepad);
  return NULL;
}

void mg_gamepad_update(mg_gamepad *gamepad) { MG_UNUSED(gamepad); }

int mg_gamepad_get_button_status(mg_gamepad *gamepad, mg_gamepad_btn btn) {
  MG_UNUSED(gamepad);
  MG_UNUSED(btn);
  return -1;
}

size_t mg_gamepad_btns_num(mg_gamepad *gamepad) {
  MG_UNUSED(gamepad);
  return 0;
}
mg_gamepad_btn mg_gamepad_btns_at(mg_gamepad *gamepad, size_t idx) {
  MG_UNUSED(gamepad);
  MG_UNUSED(idx);
  return 0;
}

size_t mg_gamepad_get_axis_num(mg_gamepad *gamepad) {
  MG_UNUSED(gamepad);
  return 0;
}

int mg_gamepad_get_axis_status(mg_gamepad *gamepad, size_t axis) {
  MG_UNUSED(gamepad);
  MG_UNUSED(axis);
  return 0;
}

mg_gamepad_axis mg_gamepad_axis_at(mg_gamepad *gamepad, size_t idx) {

  MG_UNUSED(gamepad);
  MG_UNUSED(idx);
  return 0;
}

size_t mg_gamepad_get_axis_deadzone(mg_gamepad *gamepad, size_t axis) {

  MG_UNUSED(gamepad);
  MG_UNUSED(axis);
  return 0;
}
void mg_gamepad_set_axis_deadzone(mg_gamepad *gamepad, size_t axis,
                                  size_t deadzone) {
  MG_UNUSED(gamepad);
  MG_UNUSED(axis);
  MG_UNUSED(deadzone);
  return;
}
