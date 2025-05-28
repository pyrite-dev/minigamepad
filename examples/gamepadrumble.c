#include "minigamepad.h"
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

mg_gamepads *gamepads;
mg_gamepad *gamepad;
pthread_t tid;
double axis_value;

void *rumble_thread(void *arg);

int main(void) {
  gamepads = mg_gamepads_get();
  if (gamepads->num <= 0) {
    printf("no controllers connected\n");
    return 1;
  }

  gamepad = gamepads->list[0];
  pthread_create(&tid, NULL, &rumble_thread, NULL);

  for (;;) {
    mg_gamepad_update(gamepad);
    axis_value += (double)gamepad->axises[0].value / 1000000000.0;
  }

  mg_gamepads_free(gamepads);
  return 0;
}

void *rumble_thread(void *arg) {
  while (true) {
    usleep(100);

    mg_gamepad_rumble(gamepad, axis_value / 2, axis_value, 1);
    printf("\rVibration: %0.2f", axis_value);
  }

  return 0;
}