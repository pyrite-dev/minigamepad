#include "minigamepad.h"

#ifndef __minigamepad_COMMON
#define __minigamepad_COMMON

mg_gamepad* mg_alloc(mg_gamepads* gamepads);
void mg_gamepad_free(mg_gamepad* gamepad);

void mg_gamepad_remove(mg_gamepads* gamepads, mg_gamepad* gamepad); 

/* to be defined by the backend (but used in common.c) */
void mg_gamepads_backend_init(mg_gamepads* gamepads);
void mg_gamepads_backend_free(mg_gamepads* gamepads);

#endif
