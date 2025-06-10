#include "minigamepad.h"

mg_gamepad_btn mg_get_gamepad_btn_backend(unsigned int btn) {
    (void)(btn);
    return MG_GAMEPAD_BUTTON_UNKNOWN; 
}

mg_gamepad_axis mg_get_gamepad_axis_backend(unsigned int axis) {
    (void)(axis);
    return MG_GAMEPAD_AXIS_UNKNOWN; 
}

