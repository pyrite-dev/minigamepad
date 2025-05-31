#include "minigamepad.h"
#include <stdio.h>

int main(void) {
  mg_gamepads gamepads = {0};
  mg_gamepads_init(&gamepads);

printf("\n\n");
    mg_gamepads_fetch(&gamepads);
printf("list:\n");
  for (mg_gamepad* cur = mg_gamepad_get_head(&gamepads); cur; cur = mg_gamepad_iterate(&gamepads, cur)) {
    printf("%s\n", cur->name);
  }

  mg_gamepads_free(&gamepads);
  return 0;
}
