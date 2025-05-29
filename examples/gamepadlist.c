#include "minigamepad.h"
#include <stdio.h>

int main(void) {
  mg_gamepads gamepads = {0};
  mg_gamepads_fetch(&gamepads);

  size_t joystick_num = gamepads.num;

  for (size_t i = 0; i < joystick_num; i++) {
    mg_gamepad *gamepad = mg_gamepads_at(&gamepads, i);
    printf("%s\n", mg_gamepad_get_name(gamepad));
  }

  mg_gamepads_free(&gamepads);
  return 0;
}
