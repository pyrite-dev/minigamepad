#include "minigamepad.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void clear(void) {
  // clear screen
#ifdef _WIN32
  system("cls");
#else
  system("clear");
#endif
}

int main(void) {
  mg_gamepads gamepads = {0};

  size_t idx = 0;

  clear();

  mg_gamepads_fetch(&gamepads);
  mg_gamepad *gamepad = mg_gamepads_at(&gamepads, idx);

  for (;;) {
    mg_gamepad_update(gamepad);

    if (!mg_gamepad_is_connected(gamepad)) {
      clear();
      mg_gamepads_fetch(&gamepads);
      gamepad = mg_gamepads_at(&gamepads, idx);
    }

    printf("     Gamepad: %-25s\n", mg_gamepad_get_name(gamepad));
    size_t gamepad_button_num = gamepad->button_num;
    for (size_t i = 0; i < gamepad_button_num; i++) {
      mg_gamepad_btn btn = gamepad->buttons[i].key;

      printf("     %25s:\t", mg_gamepad_btn_get_name(btn));
      printf("     %-25d\n", gamepad->buttons[i].value);
    }

    size_t axis_num = gamepad->axis_num;
    for (size_t i = 0; i < axis_num; i++) {
      mg_gamepad_axis axis = gamepad->axises[i].key;

      printf("     %25s:\t", mg_gamepad_axis_get_name(axis));
      printf("     %-25d\n", gamepad->axises[i].value);
    }
    printf("\33[0;0H");
  }

  mg_gamepads_free(&gamepads);
  return 0;
}
