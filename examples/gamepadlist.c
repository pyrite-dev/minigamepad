#include "minigamepad.h"
#include <stdio.h>

int main() {
  mg_gamepads *joysticks = mg_gamepads_get();

  size_t joystick_num = mg_gamepads_num(joysticks);

  for (int i = 0; i < joystick_num; i++) {
    mg_gamepad *joystick = mg_gamepads_at(joysticks, i);
    printf("%s\n", mg_gamepad_get_name(joystick));

    //
  }

  mg_gamepads_free(joysticks);
  return 0;
}