#include "common.h"
#include "minigamepad.h"

void mg_gamepads_init(struct mg_gamepads_t *gamepads) {
    gamepads->head = NULL;
    gamepads->cur = NULL;

    mg_gamepads_backend_init(gamepads);
    mg_gamepads_fetch(gamepads); 
}

void mg_gamepads_free(struct mg_gamepads_t *gamepads) { 
    mg_gamepad* cur;
	
	for (cur = gamepads->head; cur != NULL; cur = cur->next) {
        mg_gamepad_free(cur);
    }

    gamepads->freed.head = NULL;
    gamepads->freed.cur = NULL;
    gamepads->head = NULL;
    gamepads->cur = NULL;
}

int mg_gamepad_get_button_status(struct mg_gamepad_t *gamepad, mg_gamepad_btn btn) {
  unsigned int i;
  for (i = 0; i < gamepad->button_num; i++) {
    if (gamepad->buttons[i].key == btn) {
      return gamepad->buttons[i].value;
    }
  }
  return -1;
}

size_t mg_gamepad_btns_num(struct mg_gamepad_t *gamepad) { return gamepad->button_num; }
mg_gamepad_btn mg_gamepad_btns_at(mg_gamepad *gamepad, size_t idx) {
  return gamepad->buttons[idx].key;
}

size_t mg_gamepad_get_axis_num(struct mg_gamepad_t *gamepad) {
  return gamepad->axis_num;
}

int mg_gamepad_get_axis_status(struct mg_gamepad_t *gamepad, size_t axis) {
  unsigned int i;
  for (i = 0; i < gamepad->axis_num; i++) {
    if (gamepad->axises[i].key == axis) {
      return gamepad->axises[i].value;
    }
  }
  return -1;
}

enum mg_gamepad_axis_t mg_gamepad_axis_at(struct mg_gamepad_t *gamepad, size_t idx) {
  return gamepad->axises[idx].key;
}



struct mg_gamepad_t* mg_alloc(struct mg_gamepads_t* gamepads) {
    struct mg_gamepad_t* data;
	struct mg_gamepad_t* cur;

	if (gamepads->num >= sizeof(gamepads->__list) / sizeof(gamepads->__list[0])) {
        return NULL;
    }
    
	data = &gamepads->__list[gamepads->num]; 
    
    for (cur = gamepads->freed.head; cur != NULL; cur = cur->next) {
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

void mg_gamepad_remove(struct mg_gamepads_t* gamepads, struct mg_gamepad_t* gamepad) { 
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


size_t mg_gamepads_num(struct mg_gamepads_t *gamepads) { return gamepads->num; }

struct mg_gamepad_t *mg_gamepads_at(struct mg_gamepads_t *gamepads, size_t idx) {
  return &gamepads->__list[idx];
}



bool mg_gamepads_update(mg_gamepads* gamepads, mg_gamepad_event* ev) {
    struct mg_gamepad_t* cur;
	
	mg_gamepads_fetch(gamepads);

    for (cur = gamepads->head; cur != NULL; cur = cur->next) {
        if (mg_gamepad_update(cur, ev)) {
            return true;
        }
    }

    return false;
}
