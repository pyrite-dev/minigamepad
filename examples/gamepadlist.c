#include "minigamepad.h"
#include <stdio.h>

int main(void) {
  mg_gamepads gamepads = {0};
  mg_gamepad* cur;
  
  mg_gamepads_init(&gamepads);

  for (cur = mg_gamepad_get_head(&gamepads); cur; cur = mg_gamepad_iterate(cur)) {
    printf("%s guid: %s\n", cur->name, cur->guid);
  }

  mg_gamepads_free(&gamepads);
  return 0;
}
