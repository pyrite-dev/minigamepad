# minigamepad

Minimalist, cross-platform, C99 library for dealing with gamepads. Currently not heavily battle-tested and is only supported for Linux.

```c
#include "minigamepad.h"
#include <stdio.h>

int main(void) {
  mg_gamepads gamepads = {0};
  mg_gamepad* cur;

  mg_gamepads_init(&gamepads);

  for (cur = mg_gamepad_get_head(&gamepads); cur; cur = mg_gamepad_iterate(cur)) {
    printf("%s\n", cur->name);
  }

  mg_gamepads_free(&gamepads);
  return 0;
}
```

# Compiling

We support three build systems: Makefile, CMake, and premake. CMake in particular is designed for 3.13, the last cmake version that runs under Windows XP, allowing this to support Visual Studio 2008.

There are no external libraries and minimal internal ones. So there's no extra setup to build, other then running the relevant commands:

Make: `make` (from the root directory or any other)
CMake: `mkdir build && cd build && cmake .. # followed by the command relevant to whatever it generates`
premake5: `premake5 --file=build.lua gmake # Does not build examples`
