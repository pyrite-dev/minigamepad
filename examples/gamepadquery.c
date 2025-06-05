#include "minigamepad.h"
#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
#include <unistd.h>
#endif

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
  size_t gamepad_button_num;
  size_t axis_num;
  mg_gamepad_btn btn;
  mg_gamepad_axis axis;
  mg_gamepad* gamepad;
  size_t i;

  mg_gamepads_init(&gamepads);
  if (gamepads.head == NULL) {
     printf("no controller connected\n");
     return 0;
  }

  gamepad = gamepads.head;
  clear();

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
    gamepad_button_num = gamepad->button_num;
    for (i = 0; i < gamepad_button_num; i++) {
      btn = gamepad->buttons[i].key;

      printf("     %25s:\t", mg_gamepad_btn_get_name(btn));
      printf("     %-25d\n", gamepad->buttons[i].value);
    }

    axis_num = gamepad->axis_num;
    for (i = 0; i < axis_num; i++) {
      axis = gamepad->axises[i].key;

      printf("     %25s:\t", mg_gamepad_axis_get_name(axis));
      printf("     %-25d\n", gamepad->axises[i].value);
    }
    printf("\33[0;0H");
  }

  mg_gamepads_free(&gamepads);
  return 0;
}
