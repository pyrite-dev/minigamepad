#include "windows.h"
#include "minigamepad.h"

#include "common.h"

#define MG_UNUSED(x) (void)(x)
void mg_gamepads_backend_init(mg_gamepads* gamepads) {
    MG_UNUSED(gamepads);
}

void mg_gamepads_backend_free(mg_gamepads* gamepads) {
    MG_UNUSED(gamepads);
}

bool mg_gamepads_fetch(mg_gamepads *gamepads) {
    MG_UNUSED(gamepads);
	return false;
}

void mg_gamepad_free(mg_gamepad *gamepad) { 
    MG_UNUSED(gamepad);
}

bool mg_gamepad_update(mg_gamepad *gamepad, mg_gamepad_event* event) {
    MG_UNUSED(gamepad);
	MG_UNUSED(event);
    return false;
}

void mg_gamepad_rumble(mg_gamepad *gamepad, uint16_t strong_vibration,
                       uint16_t weak_vibration, uint16_t milliseconds) {
	MG_UNUSED(gamepad);
	MG_UNUSED(strong_vibration);
	MG_UNUSED(weak_vibration);
	MG_UNUSED(milliseconds);
}

