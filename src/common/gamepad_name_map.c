#include "minigamepad.h"

const char *mg_gamepad_btn_get_name(mg_gamepad_btn btn) {
  switch (btn) {
  case MG_GAMEPAD_BUTTON_MAX:
  case MG_GAMEPAD_BUTTON_UNKNOWN:
    return "Unknown Button";
  case MG_GAMEPAD_BUTTON_SOUTH:
    return "South Button";
  case MG_GAMEPAD_BUTTON_WEST:
    return "West Button";
  case MG_GAMEPAD_BUTTON_NORTH:
    return "North Button";
  case MG_GAMEPAD_BUTTON_EAST:
    return "East Button";
  case MG_GAMEPAD_BUTTON_BACK:
    return "Back Button";
  case MG_GAMEPAD_BUTTON_GUIDE:
    return "Guide Button";
  case MG_GAMEPAD_BUTTON_START:
    return "Start Button";
  case MG_GAMEPAD_BUTTON_LEFT_STICK:
    return "Left Stick Button";
  case MG_GAMEPAD_BUTTON_RIGHT_STICK:
    return "Right Stick Button";
  case MG_GAMEPAD_BUTTON_LEFT_SHOULDER:
    return "Left Shoulder Button";
  case MG_GAMEPAD_BUTTON_RIGHT_SHOULDER:
    return "Right Shoulder Button";
  case MG_GAMEPAD_BUTTON_MISC1:
    return "Misc Button 1";
  case MG_GAMEPAD_BUTTON_RIGHT_PADDLE1:
    return "Paddle 1 Right";
  case MG_GAMEPAD_BUTTON_LEFT_PADDLE1:
    return "Paddle 1 Left";
  case MG_GAMEPAD_BUTTON_RIGHT_PADDLE2:
    return "Paddle 2 Right";
  case MG_GAMEPAD_BUTTON_LEFT_PADDLE2:
    return "Paddle 2 Left";
  case MG_GAMEPAD_BUTTON_TOUCHPAD:
    return "Touchpad";
  case MG_GAMEPAD_BUTTON_MISC2:
    return "Misc Button 2";
  case MG_GAMEPAD_BUTTON_MISC3:
    return "Misc Button 3";
  case MG_GAMEPAD_BUTTON_MISC4:
    return "Misc Button 4";
  case MG_GAMEPAD_BUTTON_MISC5:
    return "Misc Button 5";
  case MG_GAMEPAD_BUTTON_MISC6:
    return "Misc Button 6";
  }
  return NULL;
}

const char *mg_gamepad_axis_get_name(mg_gamepad_axis axis) {
  switch (axis) {
  case MG_GAMEPAD_AXIS_UNKNOWN:
    return "Unknown Axis";
  case MG_GAMEPAD_AXIS_X:
    return "X Axis";
  case MG_GAMEPAD_AXIS_Y:
    return "Y Axis";
  case MG_GAMEPAD_AXIS_Z:
    return "Z Axis";
  case MG_GAMEPAD_AXIS_RX:
    return "RX Axis";
  case MG_GAMEPAD_AXIS_RY:
    return "RY Axis";
  case MG_GAMEPAD_AXIS_RZ:
    return "RZ Axis";
  case MG_GAMEPAD_AXIS_THROTTLE:
    return "Throttle";
  case MG_GAMEPAD_AXIS_RUDDER:
    return "Rudder";
  case MG_GAMEPAD_AXIS_WHEEL:
    return "Wheel";
  case MG_GAMEPAD_AXIS_GAS:
    return "Gas";
  case MG_GAMEPAD_AXIS_BRAKE:
    return "Brake";
  case MG_GAMEPAD_AXIS_HAT0X:
    return "Hat 0 X Axis";
  case MG_GAMEPAD_AXIS_HAT0Y:
    return "Hat 0 Y Axis";
  case MG_GAMEPAD_AXIS_HAT1X:
    return "Hat 1 X Axis";
  case MG_GAMEPAD_AXIS_HAT1Y:
    return "Hat 1 Y Axis";
  case MG_GAMEPAD_AXIS_HAT2X:
    return "Hat 2 X Axis";
  case MG_GAMEPAD_AXIS_HAT2Y:
    return "Hat 2 Y Axis";
  case MG_GAMEPAD_AXIS_HAT3X:
    return "Hat 3 X Axis";
  case MG_GAMEPAD_AXIS_HAT3Y:
    return "Hat 3 Y Axis";
  case MG_GAMEPAD_AXIS_PRESSURE:
    return "Pressure Axis";
  case MG_GAMEPAD_AXIS_DISTANCE:
    return "Distance Axis";
  case MG_GAMEPAD_AXIS_TILT_X:
    return "Tilt X Axis";
  case MG_GAMEPAD_AXIS_TILT_Y:
    return "Tilt Y Axis";
  case MG_GAMEPAD_AXIS_TOOL_WIDTH:
    return "Tool Width Axis";
  case MG_GAMEPAD_AXIS_VOLUME:
    return "Volume Axis";
  case MG_GAMEPAD_AXIS_PROFILE:
    return "Profile Axis";
  case MG_GAMEPAD_AXIS_MISC:
    return "Misc Axis";
  default: break;
  }
  return NULL;
}
