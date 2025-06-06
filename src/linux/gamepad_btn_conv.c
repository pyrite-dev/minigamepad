#include "linux.h"
#include "minigamepad.h"
#include <linux/input-event-codes.h>

#include <stdio.h>

mg_gamepad_btn mg_get_gamepad_btn_backend(unsigned int btn) {
  switch (btn) {
  case BTN_WEST:
    return MG_GAMEPAD_BUTTON_WEST;
  case BTN_A:
    return MG_GAMEPAD_BUTTON_SOUTH;
  case BTN_NORTH:
    return MG_GAMEPAD_BUTTON_NORTH;
  case BTN_EAST:
    return MG_GAMEPAD_BUTTON_EAST;
  case BTN_BACK:
    return MG_GAMEPAD_BUTTON_BACK;
  case BTN_MODE:
    return MG_GAMEPAD_BUTTON_GUIDE;
  case BTN_START:
    return MG_GAMEPAD_BUTTON_START;
  case BTN_THUMBL:
    return MG_GAMEPAD_BUTTON_LEFT_STICK;
  case BTN_THUMBR:
    return MG_GAMEPAD_BUTTON_RIGHT_STICK;
  case BTN_TL:
    return MG_GAMEPAD_BUTTON_LEFT_SHOULDER;
  case BTN_DPAD_UP:
    return MG_GAMEPAD_BUTTON_DPAD_UP;
  case BTN_DPAD_DOWN: 
    return MG_GAMEPAD_BUTTON_DPAD_DOWN;
  case BTN_DPAD_LEFT:
    return MG_GAMEPAD_BUTTON_DPAD_LEFT;
  case BTN_DPAD_RIGHT: 
    return MG_GAMEPAD_BUTTON_DPAD_RIGHT;
  case BTN_TR:
    return MG_GAMEPAD_BUTTON_RIGHT_SHOULDER;
  case BTN_TOUCH:
    return MG_GAMEPAD_BUTTON_TOUCHPAD;
  case BTN_TRIGGER_HAPPY4:
    return MG_GAMEPAD_BUTTON_RIGHT_PADDLE1;
  case BTN_TRIGGER_HAPPY6:
    return MG_GAMEPAD_BUTTON_RIGHT_PADDLE2;
  case BTN_TRIGGER_HAPPY7:
    return MG_GAMEPAD_BUTTON_LEFT_PADDLE1;
  case BTN_TRIGGER_HAPPY8:
    return MG_GAMEPAD_BUTTON_LEFT_PADDLE2;

  case BTN_SELECT:
    return MG_GAMEPAD_BUTTON_MISC1;
  case BTN_TRIGGER_HAPPY2:
    return MG_GAMEPAD_BUTTON_MISC2;
  case BTN_TRIGGER_HAPPY3:
    return MG_GAMEPAD_BUTTON_MISC3;
  case BTN_TRIGGER_HAPPY9:
    return MG_GAMEPAD_BUTTON_MISC5;
  case BTN_TRIGGER_HAPPY10:
    return MG_GAMEPAD_BUTTON_MISC6;

    case BTN_TRIGGER:     return MG_GAMEPAD_BUTTON_WEST;       // maybe map trigger as "A"
    case BTN_THUMB:       return MG_GAMEPAD_BUTTON_SOUTH;
    case BTN_THUMB2:      return MG_GAMEPAD_BUTTON_EAST;
    case BTN_TOP:         return MG_GAMEPAD_BUTTON_NORTH;
    case BTN_TOP2:        return MG_GAMEPAD_BUTTON_START;       // or whatever fits your layout
    case BTN_PINKIE:      return MG_GAMEPAD_BUTTON_LEFT_SHOULDER;
    case BTN_BASE:        return MG_GAMEPAD_BUTTON_RIGHT_SHOULDER;
    case BTN_BASE2:       return MG_GAMEPAD_BUTTON_BACK;

    case BTN_BASE3: return MG_GAMEPAD_BUTTON_BACK;
    case BTN_BASE4: return MG_GAMEPAD_BUTTON_START;
    case BTN_BASE5: return MG_GAMEPAD_BUTTON_START;
    case BTN_BASE6: return MG_GAMEPAD_BUTTON_RIGHT_STICK;
    default: 
            return MG_GAMEPAD_BUTTON_UNKNOWN;
  }
}

int get_native_btn(mg_gamepad_btn btn) {
  switch (btn) {
  case MG_GAMEPAD_BUTTON_MAX:
  case MG_GAMEPAD_BUTTON_UNKNOWN:
    return BTN_MISC;
  case MG_GAMEPAD_BUTTON_SOUTH:
    return BTN_SOUTH;
  case MG_GAMEPAD_BUTTON_WEST:
    return BTN_WEST;
  case MG_GAMEPAD_BUTTON_NORTH:
    return BTN_NORTH;
  case MG_GAMEPAD_BUTTON_EAST:
    return BTN_EAST;
  case MG_GAMEPAD_BUTTON_BACK:
    return BTN_BACK;
  case MG_GAMEPAD_BUTTON_GUIDE:
    return BTN_MODE;
  case MG_GAMEPAD_BUTTON_START:
    return BTN_START;
  case MG_GAMEPAD_BUTTON_LEFT_STICK:
    return BTN_THUMBL;
  case MG_GAMEPAD_BUTTON_RIGHT_STICK:
    return BTN_THUMBR;
  case MG_GAMEPAD_BUTTON_DPAD_UP: 
     return BTN_DPAD_UP;
  case MG_GAMEPAD_BUTTON_DPAD_DOWN: 
    return BTN_DPAD_DOWN; 
  case MG_GAMEPAD_BUTTON_DPAD_LEFT: 
     return BTN_DPAD_LEFT;
  case MG_GAMEPAD_BUTTON_DPAD_RIGHT: 
     return BTN_DPAD_RIGHT; 
  case MG_GAMEPAD_BUTTON_LEFT_SHOULDER:
    return BTN_TL;
  case MG_GAMEPAD_BUTTON_RIGHT_SHOULDER:
    return BTN_TR;
  case MG_GAMEPAD_BUTTON_TOUCHPAD:
    return BTN_TOUCH;
  case MG_GAMEPAD_BUTTON_MISC1:
    return BTN_TRIGGER_HAPPY1;
  case MG_GAMEPAD_BUTTON_MISC2:
    return BTN_TRIGGER_HAPPY2;
  case MG_GAMEPAD_BUTTON_MISC3:
    return BTN_TRIGGER_HAPPY3;
  case MG_GAMEPAD_BUTTON_MISC4:
    return BTN_TRIGGER_HAPPY4;
  case MG_GAMEPAD_BUTTON_MISC5:
    return BTN_TRIGGER_HAPPY5;
  case MG_GAMEPAD_BUTTON_MISC6:
    return BTN_TRIGGER_HAPPY6;
  default:
    return BTN_MISC;
  }
}

mg_gamepad_axis mg_get_gamepad_axis_backend(unsigned int axis) {
  switch (axis) {
  case ABS_X:
    return MG_GAMEPAD_AXIS_X;
  case ABS_Y:
    return MG_GAMEPAD_AXIS_Y;
  case ABS_Z:
    return MG_GAMEPAD_AXIS_Z;
  case ABS_RX:
    return MG_GAMEPAD_AXIS_RX;
  case ABS_RY:
    return MG_GAMEPAD_AXIS_RY;
  case ABS_RZ:
    return MG_GAMEPAD_AXIS_RZ;
  case ABS_THROTTLE:
    return MG_GAMEPAD_AXIS_THROTTLE;
  case ABS_RUDDER:
    return MG_GAMEPAD_AXIS_RUDDER;
  case ABS_WHEEL:
    return MG_GAMEPAD_AXIS_WHEEL;
  case ABS_GAS:
    return MG_GAMEPAD_AXIS_GAS;
  case ABS_BRAKE:
    return MG_GAMEPAD_AXIS_BRAKE;
  case ABS_HAT0X:
    return MG_GAMEPAD_AXIS_HAT0X;
  case ABS_HAT0Y:
    return MG_GAMEPAD_AXIS_HAT0Y;
  case ABS_HAT1X:
    return MG_GAMEPAD_AXIS_HAT1X;
  case ABS_HAT1Y:
    return MG_GAMEPAD_AXIS_HAT1Y;
  case ABS_HAT2X:
    return MG_GAMEPAD_AXIS_HAT2X;
  case ABS_HAT2Y:
    return MG_GAMEPAD_AXIS_HAT2Y;
  case ABS_HAT3X:
    return MG_GAMEPAD_AXIS_HAT3X;
  case ABS_HAT3Y:
    return MG_GAMEPAD_AXIS_HAT3Y;
  case ABS_PRESSURE:
    return MG_GAMEPAD_AXIS_PRESSURE;
  case ABS_DISTANCE:
    return MG_GAMEPAD_AXIS_DISTANCE;
  case ABS_TILT_X:
    return MG_GAMEPAD_AXIS_TILT_X;
  case ABS_TILT_Y:
    return MG_GAMEPAD_AXIS_TILT_Y;
  case ABS_TOOL_WIDTH:
    return MG_GAMEPAD_AXIS_TOOL_WIDTH;
  case ABS_VOLUME:
    return MG_GAMEPAD_AXIS_VOLUME;
  case ABS_PROFILE:
    return MG_GAMEPAD_AXIS_PROFILE;
  case ABS_MISC:
    return MG_GAMEPAD_AXIS_MISC;
  default:
    return MG_GAMEPAD_AXIS_UNKNOWN;
  }
}

int get_native_axis(mg_gamepad_axis axis) {
  switch (axis) {
  case MG_GAMEPAD_AXIS_X:
    return ABS_X;
  case MG_GAMEPAD_AXIS_Y:
    return ABS_Y;
  case MG_GAMEPAD_AXIS_Z:
    return ABS_Z;
  case MG_GAMEPAD_AXIS_RX:
    return ABS_RX;
  case MG_GAMEPAD_AXIS_RY:
    return ABS_RY;
  case MG_GAMEPAD_AXIS_RZ:
    return ABS_RZ;
  case MG_GAMEPAD_AXIS_THROTTLE:
    return ABS_THROTTLE;
  case MG_GAMEPAD_AXIS_RUDDER:
    return ABS_RUDDER;
  case MG_GAMEPAD_AXIS_WHEEL:
    return ABS_WHEEL;
  case MG_GAMEPAD_AXIS_GAS:
    return ABS_GAS;
  case MG_GAMEPAD_AXIS_BRAKE:
    return ABS_BRAKE;
  case MG_GAMEPAD_AXIS_HAT0X:
    return ABS_HAT0X;
  case MG_GAMEPAD_AXIS_HAT0Y:
    return ABS_HAT0Y;
  case MG_GAMEPAD_AXIS_HAT1X:
    return ABS_HAT1X;
  case MG_GAMEPAD_AXIS_HAT1Y:
    return ABS_HAT1Y;
  case MG_GAMEPAD_AXIS_HAT2X:
    return ABS_HAT2X;
  case MG_GAMEPAD_AXIS_HAT2Y:
    return ABS_HAT2Y;
  case MG_GAMEPAD_AXIS_HAT3X:
    return ABS_HAT3X;
  case MG_GAMEPAD_AXIS_HAT3Y:
    return ABS_HAT3Y;
  case MG_GAMEPAD_AXIS_PRESSURE:
    return ABS_PRESSURE;
  case MG_GAMEPAD_AXIS_DISTANCE:
    return ABS_DISTANCE;
  case MG_GAMEPAD_AXIS_TILT_X:
    return ABS_TILT_X;
  case MG_GAMEPAD_AXIS_TILT_Y:
    return ABS_TILT_Y;
  case MG_GAMEPAD_AXIS_TOOL_WIDTH:
    return ABS_TOOL_WIDTH;
  case MG_GAMEPAD_AXIS_VOLUME:
    return ABS_VOLUME;
  case MG_GAMEPAD_AXIS_PROFILE:
    return ABS_PROFILE;
  default:
    return ABS_MISC;
  }
}
