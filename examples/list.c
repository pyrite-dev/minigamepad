#define MG_C89
#define MG_IMPLEMENTATION
#include "minigamepad.h"

#include <stdio.h>

int main(void) {
    mg_gamepads gamepads;
    mg_gamepad* cur;

    mg_gamepads_init(&gamepads);

    for (cur = gamepads.list.head; cur; cur = cur->next) {
        printf("%s: [GUID: %s]\n", cur->name, cur->guid);
    }

    mg_gamepads_free(&gamepads);
    return 0;
}
