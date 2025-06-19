#define TG_IMPLEMENTATION
#include "tinygamepad.h"

#include <stdio.h>

int main(void) {
    tg_gamepads gamepads;
    tg_gamepads_init(&gamepads);
    
    for (tg_gamepad* cur = gamepads.list.head; cur; cur = cur->next) {
        printf("%s: [GUID: %s]\n", cur->name, cur->guid);
    }

    tg_gamepads_free(&gamepads);
}
