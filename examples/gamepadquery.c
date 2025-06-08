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
  mg_gamepad_btn btn;
  mg_gamepad_axis axis;
  mg_gamepad* gamepad;

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

    for (btn = 0; btn < MG_GAMEPAD_BUTTON_MAX; btn++) {
      if (gamepad->buttons[btn].supported == false) continue;

      printf("     %25s:\t", mg_gamepad_btn_get_name(btn));
      printf("     %-25d\n", gamepad->buttons[btn].value);
    }
    
    for (axis = 0; axis < MG_GAMEPAD_AXIS_MAX; axis++) {
      if (gamepad->axises[axis].supported == false) 
            continue;

      printf("     %25s:\t", mg_gamepad_axis_get_name(axis));
      printf("     %-25d\n", gamepad->axises[axis].value);
    }
    printf("\33[0;0H");
  }

  mg_gamepads_free(&gamepads);
  return 0;
}
