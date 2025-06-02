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

  clear();

  mg_gamepads_init(&gamepads);
  mg_gamepad* gamepad = gamepads.head;

  for (;;) {
    while(mg_gamepads_update(&gamepads, NULL));

    // This is not best practice!
    // Checking if the controller is disconnected is not the only way you should check if you need to refresh the controller list! However, better ways of doing it are a bit complex and out of the scope of this example. 
    if (!mg_gamepad_is_connected(gamepad)) {
      clear();
      mg_gamepads_fetch(&gamepads);

      gamepad = gamepad->next;
      if (gamepad == NULL)
        break;
    }

    printf("     Gamepad: %-25s\n", gamepad->name);
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
