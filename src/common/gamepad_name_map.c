#include "minigamepad.h"

const char *mg_gamepad_btn_get_name(mg_gamepad_btn btn) {
  switch (btn) {
  case MG_GAMEPAD_BUTTON_UNKNOWN:
  case MG_GAMEPAD_BUTTON_0:
    return "Button 0";
  case MG_GAMEPAD_BUTTON_1:
    return "Button 1";
  case MG_GAMEPAD_BUTTON_2:
    return "Button 2";
  case MG_GAMEPAD_BUTTON_3:
    return "Button 3";
  case MG_GAMEPAD_BUTTON_4:
    return "Button 4";
  case MG_GAMEPAD_BUTTON_5:
    return "Button 5";
  case MG_GAMEPAD_BUTTON_6:
    return "Button 6";
  case MG_GAMEPAD_BUTTON_7:
    return "Button 7";
  case MG_GAMEPAD_BUTTON_8:
    return "Button 8";
  case MG_GAMEPAD_BUTTON_9:
    return "Button 9";
  case MG_GAMEPAD_BUTTON_10:
    return "Button 10";
  case MG_GAMEPAD_BUTTON_11:
    return "Button 11";
  case MG_GAMEPAD_BUTTON_12:
    return "Button 12";
  case MG_GAMEPAD_BUTTON_13:
    return "Button 13";
  case MG_GAMEPAD_BUTTON_JOYSTICK:
    return "Joystick Button";
  case MG_GAMEPAD_BUTTON_THUMB:
    return "Thumb Button";
  case MG_GAMEPAD_BUTTON_THUMB2:
    return "Thumb Button 2";
  case MG_GAMEPAD_BUTTON_TOP:
    return "Top Button";
  case MG_GAMEPAD_BUTTON_TOP2:
    return "Top Button 2";
  case MG_GAMEPAD_BUTTON_PINKIE:
    return "Pinkie Button";
  case MG_GAMEPAD_BUTTON_BASE:
    return "Base Button 1";
  case MG_GAMEPAD_BUTTON_BASE2:
    return "Base Button 2";
  case MG_GAMEPAD_BUTTON_BASE3:
    return "Base Button 3";
  case MG_GAMEPAD_BUTTON_BASE4:
    return "Base Button 4";
  case MG_GAMEPAD_BUTTON_BASE5:
    return "Base Button 5";
  case MG_GAMEPAD_BUTTON_BASE6:
    return "Base Button 6";
  case MG_GAMEPAD_BUTTON_DEAD:
    return "Dead Button";
  case MG_GAMEPAD_BUTTON_C:
    return "Button C";

  case MG_GAMEPAD_BUTTON_Z:
    return "Z Button";
  case MG_GAMEPAD_BUTTON_TL:
    return "TL Button";
  case MG_GAMEPAD_BUTTON_TR:
    return "TR Button";
  case MG_GAMEPAD_BUTTON_TL2:
    return "TL2 Button";
  case MG_GAMEPAD_BUTTON_TR2:
    return "TR2 Button";
  case MG_GAMEPAD_BUTTON_SELECT:
    return "Select Button";
  case MG_GAMEPAD_BUTTON_START:
    return "Start Button";
  case MG_GAMEPAD_BUTTON_MODE:
    return "Mode Button";
  case MG_GAMEPAD_BUTTON_THUMBL:
    return "Thumb Button L";
  case MG_GAMEPAD_BUTTON_THUMBR:
    return "Thumb Button R";
  case MG_GAMEPAD_BUTTON_DIGI:
    return "DIGI Button";
  }
};
