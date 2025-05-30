#include "null.h"
#include "minigamepad.h"

#include <stdio.h>

#define MG_UNUSED(x) (void)(x)
void osxDeviceAddedCallback(void* context, IOReturn result, void *sender, IOHIDDeviceRef device) {
	MG_UNUSED(context); MG_UNUSED(result); MG_UNUSED(sender);
}

void osxDeviceRemovedCallback(void *context, IOReturn result, void *sender, IOHIDDeviceRef device) {
	MG_UNUSED(context); MG_UNUSED(result); MG_UNUSED(sender); MG_UNUSED(device);
}
mg_gamepads *mg_gamepads_get(void) {
    return NULL;
}

size_t mg_gamepads_num(mg_gamepads *gamepads) { return gamepads->num; }

void mg_gamepads_free(mg_gamepads *gamepads) { MG_UNUSED(gamepads); }

const char *mg_gamepad_get_name(mg_gamepad *gamepad) {
    MG_UNUSED(gamepad);
}

void mg_gamepad_update(mg_gamepad *gamepad) {
    MG_UNUSED(gamepad); 
}

int mg_gamepad_get_button_status(mg_gamepad *gamepad, mg_gamepad_btn btn) {
    MG_UNUSED(gamepad); MG_UNUSED(btn);
    return 0;
}

size_t mg_gamepad_btns_num(mg_gamepad *gamepad) { return gamepad->button_num; }
mg_gamepad_btn mg_gamepad_btns_at(mg_gamepad *gamepad, size_t idx) {
  return gamepad->buttons[idx].key;
}

size_t mg_gamepad_get_axis_num(mg_gamepad *gamepad) {
  return gamepad->axis_num;
}

int mg_gamepad_get_axis_status(mg_gamepad *gamepad, size_t axis) {
  for(unsigned int i = 0; i < gamepad->axis_num; i++) {
    if (gamepad->axises[i].key == axis) {
      return gamepad->axises[i].value;
    }
  }
  return -1;
}

mg_gamepad_axis mg_gamepad_axis_at(mg_gamepad *gamepad, size_t idx) {
  return gamepad->axises[idx].key;
}

