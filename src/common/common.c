#include "common.h"
#include "minigamepad.h"


mg_gamepad* mg_gamepad_get_head(mg_gamepads* gamepads) {
    return gamepads->head;
}

mg_gamepad* mg_gamepad_iterate(mg_gamepad* cur) {
    if (cur == NULL) {
        return NULL;
    }

    cur = cur->next;
    return (mg_gamepad*)cur;
}

void mg_gamepads_init(mg_gamepads *gamepads) {
  gamepads->head = NULL;
  gamepads->cur = NULL;

  mg_gamepads_backend_init(gamepads);
  mg_gamepads_fetch(gamepads);
}


#include <stdio.h>
void mg_gamepads_free(mg_gamepads *gamepads) {
  mg_gamepad *cur;

  for (cur = gamepads->head; cur != NULL; cur = cur->next) {
    mg_gamepad_remove(gamepads, cur);
  }

  gamepads->freed.head = NULL;
  gamepads->freed.cur = NULL;
  gamepads->head = NULL;
  gamepads->cur = NULL;
  
  mg_gamepads_backend_free(gamepads);
}

int mg_gamepad_get_button_status(mg_gamepad *gamepad,
                                 mg_gamepad_btn btn) {
  unsigned int i;
  for (i = 0; i < gamepad->button_num; i++) {
    if (gamepad->buttons[i].key == btn) {
      return gamepad->buttons[i].value;
    }
  }
  return -1;
}

size_t mg_gamepad_btns_num(mg_gamepad *gamepad) {
  return gamepad->button_num;
}
mg_gamepad_btn mg_gamepad_btns_at(mg_gamepad *gamepad, size_t idx) {
  return gamepad->buttons[idx].key;
}

size_t mg_gamepad_get_axis_num(mg_gamepad *gamepad) {
  return gamepad->axis_num;
}

int mg_gamepad_get_axis_status(mg_gamepad *gamepad, size_t axis) {
  unsigned int i;
  for (i = 0; i < gamepad->axis_num; i++) {
    if (gamepad->axises[i].key == axis) {
      return gamepad->axises[i].value;
    }
  }
  return -1;
}

mg_gamepad *mg_alloc(mg_gamepads *gamepads) {
  mg_gamepad *data;

  if (gamepads->num >= sizeof(gamepads->__list) / sizeof(gamepads->__list[0])) {
    return NULL;
  }

  data = &gamepads->__list[gamepads->num];
  if (gamepads->freed.head) {
    data = gamepads->freed.head;

    gamepads->freed.head =  gamepads->freed.head->next;
    if (gamepads->freed.head) {
        gamepads->freed.head->prev = NULL;
    } else {
        gamepads->freed.cur = NULL;
    }
  }

  if (gamepads->head == NULL) {
    gamepads->head = data;
    gamepads->head->prev = NULL;
    gamepads->cur = gamepads->head;
  } else {
    gamepads->cur->next = data;
    data->prev = gamepads->cur;
    gamepads->cur = gamepads->cur->next;
  }

  gamepads->cur->next = NULL;

  data->connected = true;
  gamepads->num++;
  return gamepads->cur;
}

void mg_gamepad_remove(mg_gamepads *gamepads,
                       mg_gamepad *gamepad) {
  if (gamepad->connected == false)
     return;
  
  gamepad->connected = false;
  /* free the gamepad's backend API data */
  mg_gamepad_free(gamepad);

  /* remove the gamepad from the linked list */
  if (gamepad->prev != NULL) {
    gamepad->prev->next = gamepad->next;
  } else if (gamepad == gamepads->head) {
    gamepads->head = gamepad->next;
  }
  if (gamepad->next != NULL) {  
     gamepad->next->prev = gamepad->prev;
  }
  
  if (gamepad == gamepads->cur) {
     gamepads->cur = gamepad->prev;
  }
  
  gamepads->num--;

  /* add the gamepad node to the freed nodes linked list */
  if (gamepads->freed.head == NULL) {
    gamepads->freed.head = gamepad;
    gamepads->freed.head->prev = NULL;
    gamepads->freed.cur = gamepads->freed.head;
  } else {
    gamepads->freed.cur->next = gamepad;
    gamepads->freed.cur->next->prev = gamepads->freed.head;
    gamepads->freed.cur = gamepads->freed.head;
  }

  gamepads->freed.cur->next = NULL;
}

size_t mg_gamepads_num(mg_gamepads *gamepads) { return gamepads->num; }

mg_gamepad *mg_gamepads_at(mg_gamepads *gamepads,
                                    size_t idx) {
  return &gamepads->__list[idx];
}

bool mg_gamepads_update(mg_gamepads *gamepads, mg_gamepad_event *ev) {
  mg_gamepad *cur;

  mg_gamepads_fetch(gamepads);

  for (cur = gamepads->head; cur != NULL; cur = cur->next) {
    if (mg_gamepad_update(cur, ev)) {
      return true;
    }
  }

  return false;
}

bool mg_gamepad_is_connected(mg_gamepad *gamepad) {
    return gamepad->connected;
}

const char *mg_gamepad_get_name(mg_gamepad *gamepad) {
    return gamepad->name;
}
