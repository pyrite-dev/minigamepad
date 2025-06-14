#include "minigamepad.h"

#ifndef __minigamepad_COMMON
#define __minigamepad_COMMON

mg_gamepad* mg_alloc(mg_gamepads* gamepads);
void mg_gamepad_free(mg_gamepad* gamepad);

void mg_gamepad_remove(mg_gamepads* gamepads, mg_gamepad* gamepad); 

/* to be defined by the backend (but used in common.c) */
void mg_gamepads_backend_init(mg_gamepads* gamepads);
void mg_gamepads_backend_free(mg_gamepads* gamepads);

mg_gamepad_btn mg_get_gamepad_btn(mg_gamepad* gamepad, unsigned int btn);
mg_gamepad_axis mg_get_gamepad_axis(mg_gamepad* gamepad, unsigned int axis);

mg_gamepad_btn mg_get_gamepad_btn_backend(unsigned int btn);
mg_gamepad_axis mg_get_gamepad_axis_backend(unsigned int axis);
//int mg_get_native_btn(mg_gamepad_btn btn);
// int mg_get_native_axis(mg_gamepad_axis axis);

void mg_mappings_init(void);

#endif
