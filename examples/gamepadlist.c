#include "minigamepad.h"
#include <stdio.h>

int main(void) {
  mg_gamepads gamepads = {0};
  mg_gamepads_init(&gamepads);

  mg_gamepads_fetch(&gamepads);
  for (mg_gamepad* cur = mg_gamepad_get_head(&gamepads); cur; cur = mg_gamepad_iterate(cur)) {
    printf("%s\n", cur->name);
  }

  mg_gamepads_free(&gamepads);
  return 0;
}
