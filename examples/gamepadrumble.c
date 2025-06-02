#include "minigamepad.h"
#include <stdio.h>
#include <time.h>

#ifndef _WIN32
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#else
#include <windows.h>
// #include <processthreadsapi.h>
// #include <handleapi.h>
// #include <synchapi.h>
#endif

mg_gamepads gamepads = {0};
mg_gamepad *gamepad;

double axis_value;

#ifndef _WIN32
pthread_t thread_id;
__attribute__((__noreturn__)) void *rumble_thread(void *arg);
#else
DWORD thread_handle;
__declspec(noreturn) DWORD WINAPI rumble_thread(LPVOID lpParam);
#endif

int main(void) {
  mg_gamepads_fetch(&gamepads);
  if (gamepads.num <= 0) {
    printf("no controllers connected\n");
    return 1;
  }

  gamepad = mg_gamepads_at(&gamepads, 0);

#ifndef _WIN32
  pthread_create(&thread_id, NULL, &rumble_thread, NULL);
#else
  CreateThread(NULL,            // default security attributes
               0,               // use default stack size
               rumble_thread,   // thread function name
               NULL,            // argument to thread function
               0,               // use default creation flags
               &thread_handle); // returns the thread identifier
#endif

  for (;;) {
    mg_gamepad_update(gamepad, NULL);
    axis_value += (double)gamepad->axises[0].value / 1000000000.0;
  }

  mg_gamepads_free(&gamepads);

#ifndef _WIN32
  pthread_cancel(thread_id);
#else
  CloseHandle(thread_handle);
#endif

  return 0;
}

#ifndef _WIN32
__attribute__((__noreturn__)) void *rumble_thread(void *arg) {
#else
__declspec(noreturn) DWORD WINAPI rumble_thread(LPVOID arg) {
#endif
  (void)(arg); // Mute warnings/errors about arg being unused

  while (true) {
    mg_gamepad_rumble(gamepad, (uint16_t)axis_value / 2, (uint16_t)axis_value,
                      1000);
    printf("\rVibration: %0.2f", axis_value);
#ifndef _WIN32
    fflush(stdout);
    sleep(1);
#else
    Sleep(1000);
#endif
  }
}
