#define TG_IMPLEMENTATION
#include "tinygamepad.h"

#include <stdio.h>

int main(void) {
    tg_gamepads gamepads;
    tg_gamepad* cur;

    tg_gamepads_init(&gamepads);

    for (cur = gamepads.list.head; cur; cur = cur->next) {
        printf("%s: [GUID: %s]\n", cur->name, cur->guid);
    }

    tg_gamepads_free(&gamepads);
    return 0;
}
