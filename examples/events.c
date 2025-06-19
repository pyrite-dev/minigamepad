#define TG_IMPLEMENTATION
#include "tinygamepad.h"

#include <stdio.h>

int main(void) {
    tg_gamepads gamepads;
    tg_gamepads_init(&gamepads);
    
    while (gamepads.list.head) {
        tg_event ev;
        while (tg_gamepads_update(&gamepads, &ev)) {
            switch (ev.type) {
                case TG_EVENT_BUTTON_PRESS:
                    printf("button press (gamepad %p) %i\n", (void*)ev.gamepad, ev.button);
                    break;
                case TG_EVENT_BUTTON_RELEASE:
                    printf("button release (gamepad %p) %i\n", (void*)ev.gamepad, ev.button);
                    break;
                case TG_EVENT_AXIS_MOVE:
                    printf("axis move (gamepad %p) %i\n", (void*)ev.gamepad, ev.axis);
                    break;
                case TG_EVENT_GAMEPAD_CONNECT:
                    printf("gamepad connected (gamepad %p)\n", (void*)ev.gamepad);
                    break;
                case TG_EVENT_GAMEPAD_DISCONNECT:
                    printf("gamepad disconnected (gamepad %p)\n", (void*)ev.gamepad);
                    break;
                default: break;
            }
        }
    }

    tg_gamepads_free(&gamepads);
}
