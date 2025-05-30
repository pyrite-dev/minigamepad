#include "common.h"
#include "minigamepad.h"

void mg_gamepads_init(mg_gamepads *gamepads) {
    gamepads->head = NULL;
    gamepads->cur = NULL;

    mg_gamepads_backend_init(gamepads);
    mg_gamepads_fetch(gamepads); 
}

void mg_gamepads_free(mg_gamepads *gamepads) { 
    for (struct mg_gamepad_t* cur = gamepads->head; cur != NULL; cur = cur->next) {
        mg_gamepad_free(cur);
    }

    gamepads->freed.head = NULL;
    gamepads->freed.cur = NULL;
    gamepads->head = NULL;
    gamepads->cur = NULL;
}

int mg_gamepad_get_button_status(mg_gamepad *gamepad, mg_gamepad_btn btn) {
  for (unsigned int i = 0; i < gamepad->button_num; i++) {
    if (gamepad->buttons[i].key == btn) {
      return gamepad->buttons[i].value;
    }
  }
  return -1;
}

size_t mg_gamepad_btns_num(mg_gamepad *gamepad) { return gamepad->button_num; }
mg_gamepad_btn mg_gamepad_btns_at(mg_gamepad *gamepad, size_t idx) {
  return gamepad->buttons[idx].key;
}

size_t mg_gamepad_get_axis_num(mg_gamepad *gamepad) {
  return gamepad->axis_num;
}

int mg_gamepad_get_axis_status(mg_gamepad *gamepad, size_t axis) {
  for (unsigned int i = 0; i < gamepad->axis_num; i++) {
    if (gamepad->axises[i].key == axis) {
      return gamepad->axises[i].value;
    }
  }
  return -1;
}

mg_gamepad_axis mg_gamepad_axis_at(mg_gamepad *gamepad, size_t idx) {
  return gamepad->axises[idx].key;
}



struct mg_gamepad_t* mg_alloc(struct mg_gamepads_t* gamepads) {
    if (gamepads->num >= sizeof(gamepads->__list) / sizeof(gamepads->__list[0])) {
        return NULL;
    }
    
    struct mg_gamepad_t* data = &gamepads->__list[gamepads->num]; 
    
    for (struct mg_gamepad_t* cur = gamepads->freed.head; cur != NULL; cur = cur->next) {
        data = cur;

        if (cur->prev != NULL) { 
            cur->prev->next = cur->next;
        }

        if (cur->next != NULL) {
            cur->next->prev = cur->prev;
        }

        break;
    }
    
    if (gamepads->head == NULL) {
        gamepads->head = data; 
        gamepads->head->prev = NULL;
        gamepads->cur = gamepads->head;  
    } else {
        gamepads->cur->next = data; 
        gamepads->cur->next->prev = gamepads->head;
        gamepads->cur = gamepads->head;  
    }

    gamepads->cur->next = NULL;

    gamepads->num++;
    return gamepads->cur;
}

void mg_gamepad_remove(mg_gamepads* gamepads, mg_gamepad* gamepad) { 
    /* free the gamepad's backend API data */
    mg_gamepad_free(gamepad);
    
    /* remove the gamepad from the linked list */ 
    if (gamepad->prev != NULL) {
        gamepad->prev->next = gamepad->next;
    } else {
        gamepads->cur = NULL;
        gamepads->head = NULL;
    }

    if (gamepad->next != NULL) {
        gamepad->next->prev = gamepad->prev;
    }

    gamepads->num--;
    
    /* add the gamepad node to the freed nodes linked list */
    if (gamepads->freed.head == NULL) {
        gamepads->freed.head = gamepad; 
        gamepads->freed.head->prev = NULL;
        gamepads->freed.cur = gamepads->head;  
    } else {
        gamepads->freed.cur->next = gamepad; 
        gamepads->freed.cur->next->prev = gamepads->head;
        gamepads->freed.cur = gamepads->head;  
    }

    gamepads->freed.cur->next = NULL;
}


size_t mg_gamepads_num(mg_gamepads *gamepads) { return gamepads->num; }

mg_gamepad *mg_gamepads_at(mg_gamepads *gamepads, size_t idx) {
  return &gamepads->__list[idx];
}



bool mg_gamepads_update(mg_gamepads* gamepads, mg_gamepad_event* ev) {
    mg_gamepads_fetch(gamepads);

    for (struct mg_gamepad_t* cur = gamepads->head; cur != NULL; cur = cur->next) {
        if (mg_gamepad_update(cur, ev)) {
            return true;
        }
    }

    return false;
}
