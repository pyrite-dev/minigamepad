#include "iokit.h"
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
    IOHIDManagerRef hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	if (!hidManager) {
		printf("Failed to create IOHIDManager.\n");
		return NULL;
	}

	CFMutableDictionaryRef matchingDictionary = CFDictionaryCreateMutable(
		kCFAllocatorDefault,
		0,
		&kCFTypeDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks
	);
	if (!matchingDictionary) {
		printf("Failed to create matching dictionary for IOKit.\n");
		CFRelease(hidManager);
		return;
	}

	CFDictionarySetValue(
		matchingDictionary,
		CFSTR(kIOHIDDeviceUsagePageKey),
		CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, (int[]){kHIDPage_GenericDesktop})
	);

	IOHIDManagerSetDeviceMatching(hidManager, matchingDictionary);

	IOHIDManagerRegisterDeviceMatchingCallback(hidManager, osxDeviceAddedCallback, NULL);
	IOHIDManagerRegisterDeviceRemovalCallback(hidManager, osxDeviceRemovedCallback, NULL);

	IOHIDManagerScheduleWithRunLoop(hidManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

	IOHIDManagerOpen(hidManager, kIOHIDOptionsTypeNone);

	/* Execute the run loop once in order to register any initially-attached joysticks */
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);
}

size_t mg_gamepads_num(mg_gamepads *gamepads) { return gamepads->num; }

void mg_gamepads_free(mg_gamepads *gamepads) { free(gamepads); }

const char *mg_gamepad_get_name(mg_gamepad *gamepad) {

}

void mg_gamepad_update(mg_gamepad *gamepad) {

}

int mg_gamepad_get_button_status(mg_gamepad *gamepad, mg_gamepad_btn btn) {

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

