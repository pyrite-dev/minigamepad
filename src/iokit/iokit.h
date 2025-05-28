#include "minigamepad.h"
#include <stdint.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDManager.h>

#ifndef __minigamepad_IOKIT
#define __minigamepad_IOKIT

struct mg_gamepad_context_t {

};

mg_gamepad_btn get_gamepad_btn(unsigned int btn);
mg_gamepad_axis get_gamepad_axis(unsigned int axis);
int get_native_btn(mg_gamepad_btn btn);
int get_native_axis(mg_gamepad_axis axis);
#endif
