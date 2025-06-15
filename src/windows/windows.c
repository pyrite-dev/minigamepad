#define WIN32_LEAN_AND_MEAN
#define OEMRESOURCE
#include <windows.h>
#include <winuser.h>


#include "mg_windows.h"
#include "minigamepad.h"

#include "common.h"
#include "sdl_db.h"

#define MG_UNUSED(x) (void)(x)

#include <assert.h>
#include <string.h>

#include <stdlib.h>
#include <stdio.h>

typedef DWORD (WINAPI * PFN_XInputGetState)(DWORD,XINPUT_STATE*);
typedef DWORD (WINAPI * PFN_XInputGetCapabilities)(DWORD,DWORD,XINPUT_CAPABILITIES*);
typedef HRESULT (WINAPI * PFN_DirectInput8Create)(HINSTANCE,DWORD,REFIID,LPVOID*,LPUNKNOWN);

typedef void (*mg_proc)(void); /* function pointer equivalent of void* */

typedef DWORD (WINAPI * PFN_XInputGetKeystroke)(DWORD, DWORD, PXINPUT_KEYSTROKE);
typedef struct mg_internal {
    HINSTANCE xinput_dll;
    PFN_XInputGetState XInputGetState;
    PFN_XInputGetKeystroke XInputGetKeystroke;
    PFN_XInputGetCapabilities XInputGetCapabilities;

    HINSTANCE dinput_dll;
    PFN_DirectInput8Create DInput8Create;
    IDirectInput8* dinput;
} mg_internal;

mg_internal internal = {0};
const GUID MG_IID_IDirectInput8W =
    {0xbf798031,0x483a,0x4da2,{0xaa,0x99,0x5d,0x64,0xed,0x36,0x97,0x00}};
const GUID MG_GUID_XAxis =
    {0xa36d02e0,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
const GUID MG_GUID_YAxis =
    {0xa36d02e1,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
const GUID MG_GUID_ZAxis =
    {0xa36d02e2,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
const GUID MG_GUID_RxAxis =
    {0xa36d02f4,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
const GUID MG_GUID_RyAxis =
    {0xa36d02f5,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
const GUID MG_GUID_RzAxis =
    {0xa36d02e3,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
const GUID MG_GUID_Slider =
    {0xa36d02e4,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};
const GUID MG_GUID_POV =
    {0xa36d02f2,0xc9f3,0x11cf,{0xbf,0xc7,0x44,0x45,0x53,0x54,0x00,0x00}};

static DIOBJECTDATAFORMAT mg_objectDataFormats[] = {
    { &MG_GUID_XAxis,DIJOFS_X,DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
    { &MG_GUID_YAxis,DIJOFS_Y,DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
    { &MG_GUID_ZAxis,DIJOFS_Z,DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
    { &MG_GUID_RxAxis,DIJOFS_RX,DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
    { &MG_GUID_RyAxis,DIJOFS_RY,DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
    { &MG_GUID_RzAxis,DIJOFS_RZ,DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
    { &MG_GUID_Slider,DIJOFS_SLIDER(0),DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
    { &MG_GUID_Slider,DIJOFS_SLIDER(1),DIDFT_AXIS|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,DIDOI_ASPECTPOSITION },
    { &MG_GUID_POV,DIJOFS_POV(0),DIDFT_POV|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { &MG_GUID_POV,DIJOFS_POV(1),DIDFT_POV|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { &MG_GUID_POV,DIJOFS_POV(2),DIDFT_POV|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { &MG_GUID_POV,DIJOFS_POV(3),DIDFT_POV|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(0),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(1),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(2),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(3),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(4),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(5),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(6),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(7),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(8),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(9),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(10),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(11),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(12),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(13),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(14),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(15),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(16),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(17),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(18),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(19),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(20),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(21),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(22),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(23),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(24),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(25),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(26),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(27),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(28),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(29),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(30),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
    { NULL,DIJOFS_BUTTON(31),DIDFT_BUTTON|DIDFT_OPTIONAL|DIDFT_ANYINSTANCE,0 },
};

const DIDATAFORMAT mg_dataFormat = {
    sizeof(DIDATAFORMAT),
    sizeof(DIOBJECTDATAFORMAT),
    DIDFT_ABSAXIS,
    sizeof(DIJOYSTATE),
    sizeof(mg_objectDataFormats) / sizeof(DIOBJECTDATAFORMAT),
    mg_objectDataFormats
};


bool mg_supportsXInput(const GUID* guid) {
    RAWINPUTDEVICELIST* list;
    unsigned int count = 0;

    if (internal.xinput_dll == NULL) {
        return false;
    }

    if (GetRawInputDeviceList(NULL, &count, sizeof(RAWINPUTDEVICELIST)) != 0)
        return false;

    list = (RAWINPUTDEVICELIST*)malloc(count * sizeof(RAWINPUTDEVICELIST));
    memset(list, 0, count * sizeof(RAWINPUTDEVICELIST));

    if ((int)GetRawInputDeviceList(list, &count, sizeof(RAWINPUTDEVICELIST)) == -1) {
        free(list);
        return false;
    }

    for (size_t i = 0;  i < count;  i++) {
        RID_DEVICE_INFO rdi = {0};
        rdi.cbSize = sizeof(rdi);

        char name[256];
        UINT size = sizeof(rdi);

        if (list[i].dwType != RIM_TYPEHID)
            continue;

        if ((int)GetRawInputDeviceInfoA(list[i].hDevice, RIDI_DEVICEINFO, &rdi, &size) == -1) {
            continue;
        }

        if (MAKELONG(rdi.hid.dwVendorId, rdi.hid.dwProductId) != (LONG) guid->Data1)
            continue;

        memset(name, 0, sizeof(name));
        size = sizeof(name);

        if ((int)GetRawInputDeviceInfoA(list[i].hDevice, RIDI_DEVICENAME, name, &size) == -1) {
            break;
        }

        name[sizeof(name) - 1] = '\0';
        if (strstr((char*)name, "IG_")) {
            free(list);
            return true;
        }
    }

    free(list);
    return false;
}

BOOL CALLBACK DirectInputEnumDevicesCallback(LPCDIDEVICEINSTANCE inst, LPVOID userData) {
    mg_gamepads* gamepads = (mg_gamepads*)userData;
    
    /* avoid clones */
    if (mg_supportsXInput(&inst->guidProduct))
        return DIENUM_CONTINUE;

    mg_gamepad* gamepad = mg_alloc(gamepads);

    gamepad->ctx = malloc(sizeof(struct mg_gamepad_context_t));

    if (FAILED(IDirectInput8_CreateDevice(internal.dinput, &inst->guidInstance, &gamepad->ctx->device, NULL))) {
        mg_gamepad_remove(gamepads, gamepad);
        return DIENUM_CONTINUE;
    }


    if (FAILED(IDirectInputDevice8_SetDataFormat(gamepad->ctx->device, &mg_dataFormat ))) {
        mg_gamepad_remove(gamepads, gamepad);
        return DIENUM_CONTINUE;
    }

    DIDEVCAPS caps = {0};
    caps.dwSize = sizeof(DIDEVCAPS);

    IDirectInputDevice8_GetCapabilities(gamepad->ctx->device, &caps);


    DIPROPDWORD dipd = {0};
    dipd.diph.dwSize = sizeof(dipd);
    dipd.diph.dwHeaderSize = sizeof(dipd.diph);
    dipd.diph.dwHow = DIPH_DEVICE;
    dipd.dwData = DIPROPAXISMODE_ABS;

    if (FAILED(IDirectInputDevice8_SetProperty(gamepad->ctx->device, DIPROP_AXISMODE, &dipd.diph))) {
        mg_gamepad_remove(gamepads, gamepad);
        return DIENUM_CONTINUE;
    }

   if (!WideCharToMultiByte(CP_UTF8, 0, (const unsigned short*)inst->tszInstanceName, -1, gamepad->name, sizeof(gamepad->name), NULL, NULL)) {
        mg_gamepad_remove(gamepads, gamepad);
        return DIENUM_STOP;
    }

    if (memcmp(&inst->guidProduct.Data4[2], "PIDVID", 6) == 0) {
        sprintf(gamepad->guid, "03000000%02x%02x0000%02x%02x000000000000",
                (uint8_t) inst->guidProduct.Data1,
                (uint8_t) (inst->guidProduct.Data1 >> 8),
                (uint8_t) (inst->guidProduct.Data1 >> 16),
                (uint8_t) (inst->guidProduct.Data1 >> 24));
    } else {
        sprintf(gamepad->guid, "05000000%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x00",
                gamepad->name[0], gamepad->name[1], gamepad->name[2], gamepad->name[3],
                gamepad->name[4], gamepad->name[5], gamepad->name[6], gamepad->name[7],
                gamepad->name[8], gamepad->name[9], gamepad->name[10]);
    }


    gamepad->mapping = mg_gamepad_find_valid_mapping(gamepad);

    DIJOYSTATE state;
    IDirectInputDevice8_GetDeviceState(gamepad->ctx->device, sizeof(state), &state);
 
    for (unsigned int i = 0; i < caps.dwButtons; i++) {
        mg_gamepad_btn key = mg_get_gamepad_btn(gamepad, i); 
        if (key == MG_GAMEPAD_BUTTON_UNKNOWN) 
            continue;

        if (gamepad->buttons[key].supported)
            continue;

        gamepad->buttons[key].supported = true;
        gamepad->buttons[key].value = 0;
    }

    for (unsigned int i = 0; i < caps.dwAxes; i++) {
        mg_gamepad_axis key = mg_get_gamepad_axis(gamepad, i); 
        if (key == MG_GAMEPAD_AXIS_UNKNOWN) 
            continue;

        if (gamepad->axises[key].supported)
            continue;

        gamepad->axises[key].supported = true;
        gamepad->axises[key].value = 0;
    }

    return DIENUM_CONTINUE;
}

void mg_gamepads_backend_init(mg_gamepads* gamepads) {
    /* init global internal data */
    if (internal.xinput_dll == NULL && internal.dinput_dll == NULL) {
        /* load xinput dll and functions (if it's available) */
        static const char* names[] = {"xinput0_4.dll", "xinput9_1_0.dll", "xinput1_2.dll", "xinput1_1.dll"};

        uint32_t i;
        for (i = 0; i < sizeof(names) / sizeof(const char*) && (internal.XInputGetState == NULL || internal.XInputGetKeystroke != NULL);  i++) {
            internal.xinput_dll = LoadLibraryA(names[i]);
            
            if (internal.xinput_dll) {
                internal.XInputGetState = (PFN_XInputGetState)(mg_proc)GetProcAddress(internal.xinput_dll, "XInputGetState");
                internal.XInputGetKeystroke = (PFN_XInputGetKeystroke)(mg_proc)GetProcAddress(internal.xinput_dll, "XInputGetKeystroke");
                internal.XInputGetCapabilities =  (PFN_XInputGetCapabilities)(mg_proc)GetProcAddress(internal.xinput_dll, "XInputGetCapabilities");

            }
        }

        /* load directinput dll and functions  */
        internal.dinput_dll = LoadLibraryA("dinput8.dll");
        if (internal.dinput_dll) {
            internal.DInput8Create = (PFN_DirectInput8Create)(mg_proc)GetProcAddress(internal.dinput_dll, "DirectInput8Create");
            HINSTANCE hInstance = GetModuleHandle(0);
            if (FAILED(internal.DInput8Create(hInstance,
                                              DIRECTINPUT_VERSION,
                                              &MG_IID_IDirectInput8W,
                                              (void**) &internal.dinput,
                                              NULL)) ||
                FAILED(IDirectInput8_EnumDevices(internal.dinput,
                                                 DI8DEVCLASS_GAMECTRL,
                                                 DirectInputEnumDevicesCallback,
                                                 (void*)gamepads,
                                                 DIEDFL_ALLDEVICES))) { 
                internal.dinput_dll = NULL; 
            }
        }
    }

    /* init gamepads data  */
}

void mg_gamepads_backend_free(mg_gamepads* gamepads) {
    MG_UNUSED(gamepads);
    if (internal.xinput_dll) {
        FreeLibrary(internal.xinput_dll);
    }

    if (internal.dinput_dll) {
        if (internal.dinput)
            IDirectInput8_Release(internal.dinput);

        FreeLibrary(internal.dinput_dll);
    }

    memset(&internal, 0, sizeof(internal));
}

bool mg_gamepads_fetch(mg_gamepads* gamepads) {
    if (internal.dinput) {
(void)(gamepads);
        /*        IDirectInput8_EnumDevices(internal.dinput,
                                  DI8DEVCLASS_GAMECTRL,
                                  DirectInputEnumDevicesCallback,
                                  (void*)gamepads,
                                  DIEDFL_ALLDEVICES);
  */
    }
    return false;
}

void mg_gamepad_free(mg_gamepad* gamepad) { 
    IDirectInputDevice8_Release(gamepad->ctx->device);
}

bool mg_gamepad_update(mg_gamepad *gamepad, mg_gamepad_event* event) {
    if (gamepad->connected == false) {
        mg_gamepad_free(gamepad);
        return false;
    }
    
    MG_UNUSED(event);
    if (internal.dinput_dll) {
        DIDEVCAPS caps = {0};
        caps.dwSize = sizeof(DIDEVCAPS);

        DIJOYSTATE state;
        HRESULT result = IDirectInputDevice8_GetDeviceState(gamepad->ctx->device, sizeof(state), &state);
        if (result == DIERR_NOTACQUIRED || result == DIERR_INPUTLOST) {
            event->type = MG_GAMEPAD_DISCONNECT;
            event->gamepad = gamepad;
            gamepad->connected = false;
            return false;
        }

        if (FAILED(result)) {
//            mg_gamepad_free(gamepad);
            return false;
        }

        for (unsigned int i = 0; i < caps.dwButtons; i++) {
            mg_gamepad_btn key = mg_get_gamepad_btn(gamepad, i); 
            if (key == MG_GAMEPAD_BUTTON_UNKNOWN) 
                continue;

            gamepad->buttons[key].value = state.rgbButtons[i];
        }
    }

    return false;
}

void mg_gamepad_rumble(mg_gamepad *gamepad, uint16_t strong_vibration,
                       uint16_t weak_vibration, uint16_t milliseconds) {
	MG_UNUSED(gamepad);
	MG_UNUSED(strong_vibration);
	MG_UNUSED(weak_vibration);
	MG_UNUSED(milliseconds);
}

