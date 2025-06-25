#define MG_IMPLEMENTATION
#include "minigamepad.h"
#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
#include <unistd.h>
#endif

void clear(void) {
  /* clear screen */
#ifdef _WIN32
  system("cls");
#else
  system("clear");
#endif
}

int main(void) {
    mg_gamepads gamepads = {0};
    mg_button btn;
    mg_axis axis;
    mg_gamepad* gamepad;

    mg_gamepads_init(&gamepads);
    if (gamepads.list.head == NULL) {
        printf("no controller connected\n");
        return 0;
    }

    gamepad = gamepads.list.head;
    clear();

    for (;;) {
        while(mg_gamepads_update(&gamepads, NULL));

        /* This is not best practice!
            Checking if the controller is disconnected is not the only way you should check if you need to refresh the controller list! However, better ways of doing it are a bit complex and out of the scope of this example. */ 
        if (!gamepad->connected) {
            clear();

            gamepad = gamepad->next;
            if (gamepad == NULL)
                break;
        }

        printf("     Gamepad: %-25s\n", gamepad->name);

        if (gamepad->buttons[MG_BUTTON_GUIDE].current == MG_TRUE) {
            break;
        }

        for (btn = 0; btn < MG_BUTTON_COUNT; btn++) {
            if (gamepad->buttons[btn].supported == MG_FALSE) continue;

            printf("     %25s:\t", mg_button_get_name(btn));
            printf("     %-25d\n", gamepad->buttons[btn].current);
        }

        for (axis = 0; axis < MG_AXIS_COUNT; axis++) {
            if (gamepad->axes[axis].supported == MG_FALSE) 
                continue;

            printf("     %25s:\t", mg_axis_get_name(axis));
            printf("     %-25f\n", gamepad->axes[axis].value);
        }
        printf("\33[0;0H");
    }
    
    printf("all controllers have been disconnected\n");
    mg_gamepads_free(&gamepads);
    return 0;
}
