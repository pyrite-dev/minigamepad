#include "minigamepad.h"
#include <stdio.h>

int main() {
  mg_gamepads *gamepads = mg_gamepads_get();

  size_t gamepad_num = mg_gamepads_num(gamepads);

  int idx = 0;

  // clear screen
  printf("\e[1;1H\e[2J");

  for (;;) {
    if (idx >= gamepad_num) {
      idx--;
      continue;
    }
    mg_gamepad *gamepad = mg_gamepads_at(gamepads, idx);

    mg_gamepad_update(gamepad);

    printf("     Gamepad: %-25s\n", mg_gamepad_get_name(gamepad));
    size_t gamepad_button_num = mg_gamepad_btns_num(gamepad);
    for (int i = 0; i < gamepad_button_num; i++) {
      mg_gamepad_btn gamepad_btn = mg_gamepad_btns_at(gamepad, i);
      if (gamepad_btn == MG_GAMEPAD_BUTTON_UNKNOWN) {
        continue;
      }
      printf("     %25s:\t", mg_gamepad_btn_get_name(gamepad_btn));
      int state = mg_gamepad_get_button_status(gamepad, gamepad_btn);
      printf("     %-25d\n", state);
    }

    size_t axis_num = mg_gamepad_get_axis_num(gamepad);
    for (int i = 0; i < axis_num; i++) {

      mg_gamepad_axis axis = mg_gamepad_axis_at(gamepad, i);

      printf("     %25s:\t", mg_gamepad_axis_get_name(axis));
      int state = mg_gamepad_get_axis_status(gamepad, axis);
      printf("     %-25d\n", state);
    }
    printf("\33[%d;%dH", 0, 0);
  }

  mg_gamepads_free(gamepads);
  return 0;
}