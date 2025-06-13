#define WIN32_LEAN_AND_MEAN
#define OEMRESOURCE
#include <windows.h>
#include <winuser.h>


#include "mg_windows.h"
#include "minigamepad.h"

#include "common.h"

#define MG_UNUSED(x) (void)(x)

#include <assert.h>
#include <string.h>

#include <xinput.h>
#include <dinput.h>
#include <stdio.h>

typedef DWORD (WINAPI * PFN_XInputGetState)(DWORD,XINPUT_STATE*);
typedef DWORD (WINAPI * PFN_XInputGetCapabilities)(DWORD,DWORD,XINPUT_CAPABILITIES*);
typedef HRESULT (WINAPI * PFN_DirectInput8Create)(HINSTANCE,DWORD,REFIID,LPVOID*,LPUNKNOWN);

typedef void (*mg_proc)(void); /* function pointer equivalent of void* */

typedef DWORD (WINAPI * PFN_XInputGetKeystroke)(DWORD, DWORD, PXINPUT_KEYSTROKE);
typedef struct mg_internal {
    HANDLE xinput_dll;
    PFN_XInputGetState XInputGetState;
    PFN_XInputGetKeystroke XInputGetKeystroke;
    PFN_XInputGetCapabilities XInputGetCapabilities;

    HANDLE dinput_dll;
    PFN_DirectInput8Create DInput8Create;
} mg_internal;

mg_internal internal = {0};

void mg_gamepads_backend_init(mg_gamepads* gamepads) {
    MG_UNUSED(gamepads);
    /* load xinput dll and functions (if it's available) */
    static const char* names[] = {"xinput0_4.dll", "xinput9_1_0.dll", "xinput1_2.dll", "xinput1_1.dll"};

    uint32_t i;
	for (i = 0; i < sizeof(names) / sizeof(const char*) && (internal.XInputGetState == NULL || internal.XInputGetKeystroke != NULL);  i++) {
		internal.xinput_dll = LoadLibraryA(names[i]);
        
        if (internal.xinput_dll) {
            internal.XInputGetState = (PFN_XInputGetState)(mg_proc)GetProcAddress(internal.xinput_dll, "XInputGetState");
            internal.XInputGetKeystroke = (PFN_XInputGetKeystroke)(mg_proc)GetProcAddress(internal.xinput_dll, "XInputGetKeystroke");
            printf("loaded xinput\n");
        }
    }

    /* load directinput dll and functions  */
	internal.dinput_dll = LoadLibraryA("dinput8.dll");
    if (internal.dinput_dll) {
        printf("loaded direct input\n");
        internal.DInput8Create = (PFN_DirectInput8Create)(mg_proc)GetProcAddress(internal.dinput_dll, "DirectInput8Create");
    }
}

void mg_gamepads_backend_free(mg_gamepads* gamepads) {
    MG_UNUSED(gamepads);
    if (internal.xinput_dll) {
        FreeLibrary(internal.xinput_dll);
    }

    if (internal.dinput_dll) {
        FreeLibrary(internal.dinput_dll);
    }   

    memset(&internal, 0, sizeof(internal));
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

