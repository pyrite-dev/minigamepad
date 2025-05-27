#ifndef __minigamepad_H
#define __minigamepad_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mg_gamepad_t mg_gamepad;
typedef struct mg_gamepads_t mg_gamepads;

/// A button on a gamepad
typedef enum {
  MG_GAMEPAD_BUTTON_0,
  MG_GAMEPAD_BUTTON_1,
  MG_GAMEPAD_BUTTON_2,
  MG_GAMEPAD_BUTTON_3,
  MG_GAMEPAD_BUTTON_4,
  MG_GAMEPAD_BUTTON_5,
  MG_GAMEPAD_BUTTON_6,
  MG_GAMEPAD_BUTTON_7,
  MG_GAMEPAD_BUTTON_8,
  MG_GAMEPAD_BUTTON_9,
  // A Button on an Xbox One Controller
  MG_GAMEPAD_BUTTON_10,
  // x Button on an Xbox One Controller
  MG_GAMEPAD_BUTTON_11,
  // Y Button on an Xbox One Controller
  MG_GAMEPAD_BUTTON_12,
  // B Button on an Xbox One Controller
  MG_GAMEPAD_BUTTON_13,

  MG_GAMEPAD_BUTTON_LEFT,
  MG_GAMEPAD_BUTTON_RIGHT,
  MG_GAMEPAD_BUTTON_MIDDLE,
  MG_GAMEPAD_BUTTON_SIDE,
  MG_GAMEPAD_BUTTON_EXTRA,
  MG_GAMEPAD_BUTTON_FORWARD,
  MG_GAMEPAD_BUTTON_BACK,
  MG_GAMEPAD_BUTTON_TASK,
  MG_GAMEPAD_BUTTON_JOYSTICK,
  MG_GAMEPAD_BUTTON_THUMB,
  MG_GAMEPAD_BUTTON_THUMB2,
  MG_GAMEPAD_BUTTON_TOP,
  MG_GAMEPAD_BUTTON_TOP2,
  MG_GAMEPAD_BUTTON_PINKIE,
  MG_GAMEPAD_BUTTON_BASE,
  MG_GAMEPAD_BUTTON_BASE2,
  MG_GAMEPAD_BUTTON_BASE3,
  MG_GAMEPAD_BUTTON_BASE4,
  MG_GAMEPAD_BUTTON_BASE5,
  MG_GAMEPAD_BUTTON_BASE6,
  MG_GAMEPAD_BUTTON_DEAD,
  MG_GAMEPAD_BUTTON_C,
  MG_GAMEPAD_BUTTON_Z,
  MG_GAMEPAD_BUTTON_TL,
  MG_GAMEPAD_BUTTON_TR,
  MG_GAMEPAD_BUTTON_TL2,
  MG_GAMEPAD_BUTTON_TR2,
  MG_GAMEPAD_BUTTON_SELECT,
  MG_GAMEPAD_BUTTON_START,
  MG_GAMEPAD_BUTTON_MODE,
  MG_GAMEPAD_BUTTON_THUMBL,
  MG_GAMEPAD_BUTTON_THUMBR,
  MG_GAMEPAD_BUTTON_DIGI,
  MG_GAMEPAD_BUTTON_UNKNOWN,
} mg_gamepad_btn;

/// Get the name of a gamepad button.
const char *mg_gamepad_btn_get_name(mg_gamepad_btn);

/// Get the gamepads currently connected to the system.
mg_gamepads *mg_gamepads_get();
/// Get the number of gamepads attached to the system.
size_t mg_gamepads_num(mg_gamepads *gamepads);
/// Get the game pad at the given index.
mg_gamepad *mg_gamepads_at(mg_gamepads *mj, size_t idx);
/// Free the struct acquired by `mg_gamepads_get`.
void mg_gamepads_free(mg_gamepads *gamepads);

/// Get the gamepad's name.
const char *mg_gamepad_get_name(mg_gamepad *gamepad);
/// Get the current status of the button.
int mg_gamepad_get_button_status(mg_gamepad *gamepad, mg_gamepad_btn btn);

/// Update the gamepad's internal structure.
/// This needs to be called before any gamepad buttons/axises are checked if you
/// want the correct values.
void mg_gamepad_update(mg_gamepad *gamepad);

/// Get the number of buttons on the given gamepad
size_t mg_gamepad_btns_num(mg_gamepad *gamepad);

/// Get the button at the given index.
mg_gamepad_btn mg_gamepad_btns_at(mg_gamepad *gamepad, size_t idx);

#ifdef __cplusplus
}
#endif

#endif