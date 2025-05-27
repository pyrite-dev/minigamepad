#ifndef __minigamepad_H
#define __minigamepad_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MG_NO_STDINT
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef uint8_t bool;
#else
#include <stdbool.h>
#include <stdint.h>
#endif

typedef struct mg_gamepad_t mg_gamepad;
typedef struct mg_gamepads_t mg_gamepads;

/// A button on a gamepad
typedef enum {
  MG_GAMEPAD_BUTTON_UNKNOWN = 0,
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
  MG_GAMEPAD_AXIS_UNKNOWN = 0,
  MG_GAMEPAD_AXIS_X,
  MG_GAMEPAD_AXIS_Y,
  MG_GAMEPAD_AXIS_Z,
  MG_GAMEPAD_AXIS_RX,
  MG_GAMEPAD_AXIS_RY,
  MG_GAMEPAD_AXIS_RZ,
  MG_GAMEPAD_AXIS_THROTTLE,
  MG_GAMEPAD_AXIS_RUDDER,
  MG_GAMEPAD_AXIS_WHEEL,
  MG_GAMEPAD_AXIS_GAS,
  MG_GAMEPAD_AXIS_BRAKE,
  MG_GAMEPAD_AXIS_HAT0X,
  MG_GAMEPAD_AXIS_HAT0Y,
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

#define MAX_BUTTONS MG_GAMEPAD_BUTTON_MAX
#define MAX_AXISES MG_GAMEPAD_AXIS_MAX

/// Internal context of the gamepad, i.e. implementation details.
struct mg_gamepad_context_t;

typedef struct mg_gamepad_t {
  // Internal context for platform dependent items.
  struct mg_gamepad_context_t *ctx;
  // Map of buttons that the controller has
  struct {
    mg_gamepad_btn key;
    int16_t value;
  } buttons[MAX_BUTTONS];
  // The number of buttons on the controller.
  size_t button_num;
  // Map of axises that the controller has, + their deadzones
  // By default, the deadzones are 5000 for any axis that isn't the d-pad; you
  // are strongly encouraged to make this customizable in any program you make
  // with this.
  struct {
    mg_gamepad_axis key;
    int16_t value;
    int16_t deadzone;
  } axises[MAX_AXISES];
  // The number of axises on the controller.
  size_t axis_num;
} mg_gamepad;

/// A list of gamepads recognized by the system.
typedef struct mg_gamepads_t {
  struct mg_gamepad_t *list[16];
  size_t num;
} mg_gamepads;

/// Update the gamepad's internal structure.
/// This needs to be called before any gamepad buttons/axises are checked if you
/// want the correct values.
void mg_gamepad_update(mg_gamepad *gamepad);

/// Get the gamepads currently connected to the system.
mg_gamepads *mg_gamepads_get(void);
/// Get the number of gamepads attached to the system.
size_t mg_gamepads_num(mg_gamepads *gamepads);
/// Get the game pad at the given index.
mg_gamepad *mg_gamepads_at(mg_gamepads *mj, size_t idx);
/// Free the struct acquired by `mg_gamepads_get`.
void mg_gamepads_free(mg_gamepads *gamepads);

/// Get the gamepad's name.
const char *mg_gamepad_get_name(mg_gamepad *gamepad);

/// Get the name of a gamepad button.
const char *mg_gamepad_btn_get_name(mg_gamepad_btn);
/// Get the name of a gamepad axis.
const char *mg_gamepad_axis_get_name(mg_gamepad_axis axis);

#ifdef __cplusplus
}
#endif

#endif
