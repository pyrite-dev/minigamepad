#include "minigamepad.h"
#include <stdio.h>

int main() {
  mg_gamepads *gamepads = mg_gamepads_get();

  size_t num = gamepads->num;

  int idx = 0;

  // clear screen
  printf("\e[1;1H\e[2J");

  for (;;) {
    if (idx >= num) {
      idx--;
      continue;
    }
    mg_gamepad *gamepad = gamepads->list[idx];
    mg_gamepad_update(gamepad);

    printf("     Gamepad: %-25s\n", mg_gamepad_get_name(gamepad));
    size_t gamepad_button_num = gamepad->button_num;
    for (int i = 0; i < gamepad_button_num; i++) {
      mg_gamepad_btn btn = gamepad->buttons[i].key;

      printf("     %25s:\t", mg_gamepad_btn_get_name(btn));
      printf("     %-25d\n", gamepad->buttons[i].value);
    }

    size_t axis_num = gamepad->axis_num;
    for (int i = 0; i < axis_num; i++) {
      mg_gamepad_axis axis = gamepad->axises[i].key;

      printf("     %25s:\t", mg_gamepad_axis_get_name(axis));
      printf("     %-25d\n", gamepad->axises[i].value);
    }
    printf("\33[%d;%dH", 0, 0);
  }

  mg_gamepads_free(gamepads);
  return 0;
}