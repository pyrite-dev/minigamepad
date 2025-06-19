#define MG_IMPLEMENTATION
#include "minigamepad.h"

#include <stdio.h>

int main(void) {
    mg_gamepads gamepads;
    mg_gamepads_init(&gamepads);
    
    while (gamepads.list.head) {
        mg_event ev;
        while (mg_gamepads_update(&gamepads, &ev)) {
            switch (ev.type) {
                case MG_EVENT_BUTTON_PRESS:
                    printf("button press (gamepad %p) %i\n", (void*)ev.gamepad, ev.button);
                    break;
                case MG_EVENT_BUTTON_RELEASE:
                    printf("button release (gamepad %p) %i\n", (void*)ev.gamepad, ev.button);
                    break;
                case MG_EVENT_AXIS_MOVE:
                    if (ev.button == MG_BUTTON_GUIDE)
                        goto end;

                    printf("axis move (gamepad %p) %i\n", (void*)ev.gamepad, ev.axis);
                    break;
                case MG_EVENT_GAMEPAD_CONNECT:
                    printf("gamepad connected (gamepad %p)\n", (void*)ev.gamepad);
                    break;
                case MG_EVENT_GAMEPAD_DISCONNECT:
                    printf("gamepad disconnected (gamepad %p)\n", (void*)ev.gamepad);
                    break;
                default: break;
            }
        }
    }
    end:

    mg_gamepads_free(&gamepads);
    return 0;
}
