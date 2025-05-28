#include "minigamepad.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#ifndef __WIN32
#include <pthread.h>
#else
#include <windows.h>
#endif

mg_gamepads *gamepads;
mg_gamepad *gamepad;

double axis_value;

#ifndef __WIN32
pthread_t thread_id;
void *rumble_thread(void *arg);
#else
HANDLE thread_handle;
DWORD WINAPI rumble_thread(LPVOID lpParam);
#endif

int main(void) {
  gamepads = mg_gamepads_get();
  if (gamepads->num <= 0) {
    printf("no controllers connected\n");
    return 1;
  }

  gamepad = gamepads->list[0];

#ifndef __WIN32
  pthread_create(&thread_id, NULL, &rumble_thread, NULL);
#else
  thread_handle =
      CreateThread(NULL,                 // default security attributes
                   0,                    // use default stack size
                   rumble_thread,        // thread function name
                   pDataArray[i],        // argument to thread function
                   0,                    // use default creation flags
                   &dwThreadIdArray[i]); // returns the thread identifier
#endif

  for (;;) {
    mg_gamepad_update(gamepad);
    axis_value += (double)gamepad->axises[0].value / 1000000000.0;
  }

  mg_gamepads_free(gamepads);

#ifndef __WIN32
  pthread_cancel(thread_id);
#else
  CloseHandle(thread_id);
#endif

  return 0;
}

#ifndef __WIN32
void *rumble_thread(void *arg) {
#else
DWORD WINAPI rumble_thread(LPVOID lpParam) {
#endif
  while (true) {
    mg_gamepad_rumble(gamepad, axis_value / 2, axis_value, 100);
    printf("\rVibration: %0.2f", axis_value);
#ifndef __WIN32
    usleep(100000);
#else
    Sleep(100);
#endif
  }

  return 0;
}