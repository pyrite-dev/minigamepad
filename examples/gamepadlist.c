#include "minigamepad.h"
#include <stdio.h>

int main(void) {
  mg_gamepads *joysticks = mg_gamepads_get();

  size_t joystick_num = mg_gamepads_num(joysticks);

  for (size_t i = 0; i < joystick_num; i++) {
    mg_gamepad *gamepad = mg_gamepads_at(joysticks, i);
    printf("%s\n", mg_gamepad_get_name(gamepad));
  }

  mg_gamepads_free(joysticks);
  return 0;
}
