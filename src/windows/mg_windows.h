#include "minigamepad.h"

#include <xinput.h>
#include <dinput.h>

#ifndef __MINIGAMEPAD_WINDOWS
#define __MINIGAMEPAD_WINDOWS

typedef struct mg_gamepad_context_t {
    IDirectInputDevice8* device;
} mg_gamepad_context_t;

#endif
