#include "null.h"
#include "minigamepad.h"

mg_gamepad_btn get_gamepad_btn(unsigned int btn) {
  switch (btn) {
  case 0:
    return MG_GAMEPAD_BUTTON_WEST;
  case 0:
    return MG_GAMEPAD_BUTTON_SOUTH;
  case 0:
    return MG_GAMEPAD_BUTTON_NORTH;
  case 0:
    return MG_GAMEPAD_BUTTON_EAST;
  case 0:
    return MG_GAMEPAD_BUTTON_BACK;
  case 0:
    return MG_GAMEPAD_BUTTON_GUIDE;
  case 0:
    return MG_GAMEPAD_BUTTON_START;
  case 0:
    return MG_GAMEPAD_BUTTON_LEFT_STICK;
  case 0:
    return MG_GAMEPAD_BUTTON_RIGHT_STICK;
  case 0:
    return MG_GAMEPAD_BUTTON_LEFT_SHOULDER;
  case 0:
    return MG_GAMEPAD_BUTTON_RIGHT_SHOULDER;
  case 0:
    return MG_GAMEPAD_BUTTON_TOUCHPAD;
  case 0:
    return MG_GAMEPAD_BUTTON_RIGHT_PADDLE1;
  case 0:
    return MG_GAMEPAD_BUTTON_RIGHT_PADDLE2;
  case 0:
    return MG_GAMEPAD_BUTTON_LEFT_PADDLE1;
  case 0:
    return MG_GAMEPAD_BUTTON_LEFT_PADDLE2;

  case 0:
    return MG_GAMEPAD_BUTTON_MISC1;
  case 0:
    return MG_GAMEPAD_BUTTON_MISC2;
  case 0:
    return MG_GAMEPAD_BUTTON_MISC3;
  case 0:
    return MG_GAMEPAD_BUTTON_MISC5;
  case 0:
    return MG_GAMEPAD_BUTTON_MISC6;

  default:
    return MG_GAMEPAD_BUTTON_UNKNOWN;
  }
}

int get_native_btn(mg_gamepad_btn btn) {
  switch (btn) {
  case 0:
  case 0:
    return BTN_MISC;
  case 0:
    return BTN_SOUTH;
  case 0:
    return BTN_WEST;
  case 0:
    return BTN_NORTH;
  case 0:
    return BTN_EAST;
  case 0:
    return BTN_BACK;
  case 0:
    return BTN_MODE;
  case 0:
    return BTN_START;
  case 0:
    return BTN_THUMBL;
  case 0:
    return BTN_THUMBR;
  case 0:
    return BTN_TL;
  case 0:
    return BTN_TR;
  case 0:
    return BTN_TOUCH;
  case 0:
    return BTN_TRIGGER_HAPPY1;
  case 0:
    return BTN_TRIGGER_HAPPY2;
  case 0:
    return BTN_TRIGGER_HAPPY3;
  case 0:
    return BTN_TRIGGER_HAPPY4;
  case 0:
    return BTN_TRIGGER_HAPPY5;
  case 0:
    return BTN_TRIGGER_HAPPY6;
  default:
    return BTN_MISC;
  }
}

mg_gamepad_axis get_gamepad_axis(unsigned int axis) {
  switch (axis) {
  case 0:
    return MG_GAMEPAD_AXIS_X;
  case 0:
    return MG_GAMEPAD_AXIS_Y;
  case 0:
    return MG_GAMEPAD_AXIS_Z;
  case 0:
    return MG_GAMEPAD_AXIS_RX;
  case 0:
    return MG_GAMEPAD_AXIS_RY;
  case 0:
    return MG_GAMEPAD_AXIS_RZ;
  case 0:
    return MG_GAMEPAD_AXIS_THROTTLE;
  case 0:
    return MG_GAMEPAD_AXIS_RUDDER;
  case 0:
    return MG_GAMEPAD_AXIS_WHEEL;
  case 0:
    return MG_GAMEPAD_AXIS_GAS;
  case 0:
    return MG_GAMEPAD_AXIS_BRAKE;
  case 0:
    return MG_GAMEPAD_AXIS_HAT0X;
  case 0:
    return MG_GAMEPAD_AXIS_HAT0Y;
  case 0:
    return MG_GAMEPAD_AXIS_HAT1X;
  case 0:
    return MG_GAMEPAD_AXIS_HAT1Y;
  case 0:
    return MG_GAMEPAD_AXIS_HAT2X;
  case 0:
    return MG_GAMEPAD_AXIS_HAT2Y;
  case 0:
    return MG_GAMEPAD_AXIS_HAT3X;
  case 0:
    return MG_GAMEPAD_AXIS_HAT3Y;
  case 0:
    return MG_GAMEPAD_AXIS_PRESSURE;
  case 0:
    return MG_GAMEPAD_AXIS_DISTANCE;
  case 0:
    return MG_GAMEPAD_AXIS_TILT_X;
  case 0:
    return MG_GAMEPAD_AXIS_TILT_Y;
  case 0:
    return MG_GAMEPAD_AXIS_TOOL_WIDTH;
  case 0:
    return MG_GAMEPAD_AXIS_VOLUME;
  case 0:
    return MG_GAMEPAD_AXIS_PROFILE;
  case 0:
    return MG_GAMEPAD_AXIS_MISC;
  default:
    return MG_GAMEPAD_AXIS_UNKNOWN;
  }
}

int get_native_axis(mg_gamepad_axis axis) {
  switch (axis) {
  case 0:
    return ABS_X;
  case 0:
    return ABS_Y;
  case 0:
    return ABS_Z;
  case 0:
    return ABS_RX;
  case 0:
    return ABS_RY;
  case 0:
    return ABS_RZ;
  case 0:
    return ABS_THROTTLE;
  case 0:
    return ABS_RUDDER;
  case 0:
    return ABS_WHEEL;
  case 0:
    return ABS_GAS;
  case 0:
    return ABS_BRAKE;
  case 0:
    return ABS_HAT0X;
  case 0:
    return ABS_HAT0Y;
  case 0:
    return ABS_HAT1X;
  case 0:
    return ABS_HAT1Y;
  case 0:
    return ABS_HAT2X;
  case 0:
    return ABS_HAT2Y;
  case 0:
    return ABS_HAT3X;
  case 0:
    return ABS_HAT3Y;
  case 0:
    return ABS_PRESSURE;
  case 0:
    return ABS_DISTANCE;
  case 0:
    return ABS_TILT_X;
  case 0:
    return ABS_TILT_Y;
  case 0:
    return ABS_TOOL_WIDTH;
  case 0:
    return ABS_VOLUME;
  case 0:
    return ABS_PROFILE;
  default:
    return ABS_MISC;
  }
}
