# minigamepad

![THE MINIGAMEPAD Logo](https://github.com/ColleagueRiley/minigamepad/blob/main/logo.png?raw=true)

![workflow](https://github.com/ColleagueRiley/minigamepad/actions/workflows/linux.yml/badge.svg)
![workflow windows](https://github.com/ColleagueRiley/minigamepad/actions/workflows/windows.yml/badge.svg)
![workflow macOS](https://github.com/ColleagueRiley/minigamepad/actions/workflows/macos.yml/badge.svg)
![workflow wasm](https://github.com/ColleagueRiley/minigamepad/actions/workflows/web.yml/badge.svg)

A simple, cross platform lightweight single-header gamepad library for handling controller input.

Written with support for C89, using overridable C99 features, eg. `stdint.h` and `inline` with `#define MG_C89`.

## progress / TODO 
### linux:
full support except for rumble 

### windows
directinput backend is started, but not complete. 

xinput backend has not yet been started other than linking.

### macos
the iokit backend has not been started 

## wasm
the emscripten backend has not been started

## basic example

```c
#define MG_IMPLEMENTATION
#include "minigamepad.h"
#include <stdio.h>

int main(void) {
    mg_gamepads gamepads = {0};
    mg_gamepad* cur;

    mg_gamepads_init(&gamepads);
    
    while (mg_gamepads_update(&gamepads, NULL));

    for (cur = gamepads.list.head; cur; cur = cur->next) {
        printf("%s\n", cur->name);
    }

    mg_gamepads_free(&gamepads);
    return 0;
}
```

Then compile with:

linux `gcc main.c` \
macos: `gcc main.c -framework IOKit` \
windows: `gcc main.c` OR `cl main.c` \
wasm: `emcc main.c -s ASYNCIFY`  

