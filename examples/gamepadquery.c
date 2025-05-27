#include "minigamepad.h"
#include <stdio.h>

int main() {
  mg_gamepads *joysticks = mg_gamepads_get();

  size_t joystick_num = mg_gamepads_num(joysticks);

  int idx = 0;

  // clear screen
  printf("\e[1;1H\e[2J");

  for (;;) {
    if (idx >= joystick_num) {
      idx--;
      continue;
    }
    mg_gamepad *joystick = mg_gamepads_at(joysticks, idx);

    mg_gamepad_update(joystick);

    printf("     Joystick: %-25s\n", mg_gamepad_get_name(joystick));
    size_t joystick_button_num = mg_gamepad_btns_num(joystick);
    for (int i = 0; i < joystick_button_num; i++) {
      mg_gamepad_btn gamepad_btn = mg_gamepad_btns_at(joystick, i);
      if (gamepad_btn == MG_GAMEPAD_BUTTON_UNKNOWN) {
        continue;
      }
      printf("     %25s:\t", mg_gamepad_btn_get_name(gamepad_btn));
      int state = mg_gamepad_get_value(joystick, gamepad_btn);
      printf("     %-25d\n", state);
    }

    printf("\33[%d;%dH", 0, 0);
  }

  mg_gamepads_free(joysticks);
  return 0;
}