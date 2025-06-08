#ifndef __minigamepad_H
#define __minigamepad_H

#if defined(_WIN32) && !defined(MG_API)
    #if defined(__TINYC__)
        #define __declspec(x) __attribute__((x))
    #endif
    #if defined(BUILD_LIBTYPE_SHARED)
        #define MG_API __declspec(dllexport)     // We are building the library as a Win32 shared library (.dll)
    #elif defined(USE_LIBTYPE_SHARED)
        #define MG_API __declspec(dllimport)     // We are using the library as a Win32 shared library (.dll)
    #endif
#elif !defined(MG_API)
    #if defined(BUILD_LIBTYPE_SHARED)
        #define MG_API __attribute__((visibility("default"))) // We are building as a Unix shared library (.so/.dylib)
    #endif
#endif

#ifndef MG_API 
    #define MG_API 
#endif

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MG_NO_STDTYPES
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef uint8_t bool;
#define false 0
#define true 0
#else
#include <stdbool.h>
#include <stdint.h>
#endif

/// A button on a gamepad
typedef enum {
  MG_GAMEPAD_BUTTON_UNKNOWN = -1,
  MG_GAMEPAD_BUTTON_SOUTH, /**< Bottom face button (e.g. Xbox A button) */
  MG_GAMEPAD_BUTTON_WEST,  /**< Left face button (e.g. Xbox X button) */
  MG_GAMEPAD_BUTTON_NORTH, /**< Top face button (e.g. Xbox Y button) */
  MG_GAMEPAD_BUTTON_EAST,  /**< Right face button (e.g. Xbox B button) */
  MG_GAMEPAD_BUTTON_BACK,
  MG_GAMEPAD_BUTTON_GUIDE,
  MG_GAMEPAD_BUTTON_START,
  MG_GAMEPAD_BUTTON_LEFT_STICK,
  MG_GAMEPAD_BUTTON_RIGHT_STICK,
  MG_GAMEPAD_BUTTON_LEFT_SHOULDER,
  MG_GAMEPAD_BUTTON_RIGHT_SHOULDER,
  MG_GAMEPAD_BUTTON_LEFT_TRIGGER,
  MG_GAMEPAD_BUTTON_RIGHT_TRIGGER,
  MG_GAMEPAD_BUTTON_DPAD_UP,
  MG_GAMEPAD_BUTTON_DPAD_DOWN,
  MG_GAMEPAD_BUTTON_DPAD_LEFT,
  MG_GAMEPAD_BUTTON_DPAD_RIGHT,
/* extras */
  MG_GAMEPAD_BUTTON_MISC1, /**< Additional button (e.g. Xbox Series X share
                               button, PS5 microphone button, Nintendo Switch
                               Pro capture button, Amazon Luna microphone
                               button, Google Stadia capture button) */
  MG_GAMEPAD_BUTTON_RIGHT_PADDLE1, /**< Upper or primary paddle, under your
                                       right hand (e.g. Xbox Elite paddle P1) */
  MG_GAMEPAD_BUTTON_LEFT_PADDLE1,  /**< Upper or primary paddle, under your left
                                       hand (e.g. Xbox Elite paddle P3) */
  MG_GAMEPAD_BUTTON_RIGHT_PADDLE2, /**< Lower or secondary paddle, under your
                                       right hand (e.g. Xbox Elite paddle P2) */
  MG_GAMEPAD_BUTTON_LEFT_PADDLE2,  /**< Lower or secondary paddle, under your
                                       left hand (e.g. Xbox Elite paddle P4) */
  MG_GAMEPAD_BUTTON_TOUCHPAD,      /**< PS4/PS5 touchpad button */
  MG_GAMEPAD_BUTTON_MISC2,         /**< Additional button */
  MG_GAMEPAD_BUTTON_MISC3,         /**< Additional button */
  MG_GAMEPAD_BUTTON_MISC4,         /**< Additional button */
  MG_GAMEPAD_BUTTON_MISC5,         /**< Additional button */
  MG_GAMEPAD_BUTTON_MISC6,         /**< Additional button */
  MG_GAMEPAD_BUTTON_MAX
} mg_gamepad_btn;

/// An axis on a gamepad
typedef enum {
  MG_GAMEPAD_AXIS_UNKNOWN = -1,
  MG_GAMEPAD_AXIS_LEFT_X,
  MG_GAMEPAD_AXIS_LEFT_Y,
  MG_GAMEPAD_AXIS_RIGHT_X,
  MG_GAMEPAD_AXIS_RIGHT_Y,
  MG_GAMEPAD_AXIS_LEFT_TRIGGER,
  MG_GAMEPAD_AXIS_RIGHT_TRIGGER,
  MG_GAMEPAD_AXIS_HAT_DPAD_LEFT_RIGHT,
  MG_GAMEPAD_AXIS_HAT_DPAD_LEFT = MG_GAMEPAD_AXIS_HAT_DPAD_LEFT_RIGHT,
  MG_GAMEPAD_AXIS_HAT_DPAD_RIGHT = MG_GAMEPAD_AXIS_HAT_DPAD_LEFT_RIGHT,
  MG_GAMEPAD_AXIS_HAT_DPAD_UP_DOWN,
  MG_GAMEPAD_AXIS_HAT_DPAD_UP = MG_GAMEPAD_AXIS_HAT_DPAD_UP_DOWN,
  MG_GAMEPAD_AXIS_HAT_DPAD_DOWN = MG_GAMEPAD_AXIS_HAT_DPAD_UP_DOWN,
  /* extras */
  MG_GAMEPAD_AXIS_THROTTLE,
  MG_GAMEPAD_AXIS_RUDDER,
  MG_GAMEPAD_AXIS_WHEEL,
  MG_GAMEPAD_AXIS_GAS,
  MG_GAMEPAD_AXIS_BRAKE,
  MG_GAMEPAD_AXIS_HAT1X,
  MG_GAMEPAD_AXIS_HAT1Y,
  MG_GAMEPAD_AXIS_HAT2X,
  MG_GAMEPAD_AXIS_HAT2Y,
  MG_GAMEPAD_AXIS_HAT3X,
  MG_GAMEPAD_AXIS_HAT3Y,
  MG_GAMEPAD_AXIS_PRESSURE,
  MG_GAMEPAD_AXIS_DISTANCE,
  MG_GAMEPAD_AXIS_TILT_X,
  MG_GAMEPAD_AXIS_TILT_Y,
  MG_GAMEPAD_AXIS_TOOL_WIDTH,
  MG_GAMEPAD_AXIS_VOLUME,
  MG_GAMEPAD_AXIS_PROFILE,
  MG_GAMEPAD_AXIS_MISC,

  MG_GAMEPAD_AXIS_MAX,
} mg_gamepad_axis;

typedef enum {
    MG_HAT_CENTERED = 0,
    MG_HAT_UP = 1,  
    MG_HAT_RIGHT = 2,
    MG_HAT_DOWN = 4,
    MG_HAT_LEFT = 8
} mg_gamepad_hat;

#define MG_MAX_HATS 2
#define MG_MAX_BUTTONS MG_GAMEPAD_BUTTON_MAX
#define MG_MAX_AXISES MG_GAMEPAD_AXIS_MAX

/// Internal context of the gamepad, i.e. implementation details.
struct mg_gamepad_context_t;

typedef struct mg_buttons {
    bool supported;
    int16_t value;
} mg_buttons;

typedef struct mg_axises {
    bool supported;
    int16_t value;
    int16_t deadzone;
} mg_axises;

struct mg_mapping;

typedef struct mg_gamepad {
    // Internal context for platform dependent items.
    struct mg_gamepad_context_t* ctx;
    // Map of buttons that the controller has
    mg_buttons buttons[MG_MAX_BUTTONS];

    // Map of axises that the controller has, + their deadzones
    // By default, the deadzones are 5000 for any axis that isn't the d-pad; you
    // are strongly encouraged to make this customizable in any program you make
    // with this.

    mg_axises axises[MG_MAX_AXISES];
    
    bool connected;
    char name[128];
    char guid[33];

    struct mg_mapping* mapping;

    struct mg_gamepad* prev;
    struct mg_gamepad* next;
} mg_gamepad;

/// A list of gamepads recognized by the system.
typedef struct mg_gamepads {
    mg_gamepad __list[16];
    mg_gamepad* cur;
    mg_gamepad* head;
    
    struct {
        mg_gamepad* cur;
        mg_gamepad* head;
    } freed;

    size_t num;
} mg_gamepads;


typedef enum mg_gamepad_event_type {
    MG_GAMEPAD_NONE = 0,
    MG_GAMEPAD_CONNECT,
    MG_GAMEPAD_DISCONNECT,
    MG_GAMEPAD_BTN_PRESS,
    MG_GAMEPAD_BTN_RELEASE,
    MG_GAMEPAD_AXIS_MOVE
} mg_gamepad_event_type;

typedef struct mg_gamepad_event {
    mg_gamepad_event_type type;
    mg_gamepad_btn btn;
    mg_gamepad_axis axis;

    mg_gamepad* gamepad;
} mg_gamepad_event; 


// get the head gamepad in the linked list of gamepads
MG_API mg_gamepad* mg_gamepad_get_head(mg_gamepads* gamepads);

// iterates to the next node in the linked list of gamepads, returns `cur`, allowing you to check if cur == NULL 
MG_API mg_gamepad* mg_gamepad_iterate(mg_gamepad* cur);

/// Init the gamepads internal structure.
MG_API void mg_gamepads_init(mg_gamepads *gamepad);

/// Fetch and update all gamepads and return the first event
MG_API bool mg_gamepads_update(mg_gamepads* gamepads, mg_gamepad_event* ev);

/// Add another mapping to the gamepads mappings
MG_API bool mg_update_gamepad_mappings(mg_gamepads*gamepads, const char* string);

/// Update the gamepad's internal structure.
/// This needs to be called before any gamepad buttons/axises are checked if you
/// want the correct values.
MG_API bool mg_gamepad_update(mg_gamepad *gamepad, mg_gamepad_event* ev);

/// Get the gamepads currently connected to the system. Returns true if a new gamepad was found
MG_API bool mg_gamepads_fetch(mg_gamepads *);
/// Get the number of gamepads attached to the system.
MG_API size_t mg_gamepads_num(mg_gamepads *gamepads);
/// Get the game pad at the given index.
MG_API mg_gamepad *mg_gamepads_at(mg_gamepads *mj, size_t idx);
/// Free gamepads.
MG_API void mg_gamepads_free(mg_gamepads *gamepads);

/// Get the gamepad's name.
MG_API const char *mg_gamepad_get_name(mg_gamepad *gamepad);
/// Check if this gamepad is connected.
MG_API bool mg_gamepad_is_connected(mg_gamepad *gamepad);

/// Get the name of a gamepad button.
MG_API const char *mg_gamepad_btn_get_name(mg_gamepad_btn);
/// Get the name of a gamepad axis.
MG_API const char *mg_gamepad_axis_get_name(mg_gamepad_axis axis);

/// Rumble the controller
MG_API void mg_gamepad_rumble(mg_gamepad *gamepad, uint16_t strong_vibration, uint16_t weak_vibration, uint16_t milliseconds);

#ifdef __cplusplus
}
#endif

#endif
