#ifndef __SDL_DB
#define __SDL_DB

#include "minigamepad.h"
extern const char* sdl_db[];

typedef struct mg_element {
    unsigned char         type;
    unsigned char         index;
    signed char          axisScale;
    signed char          axisOffset;
} mg_element;

typedef struct mg_mapping {
    char            name[128];
    char            guid[33];
    mg_element buttons[MG_GAMEPAD_BUTTON_MAX];
    mg_element axes[MG_GAMEPAD_AXIS_MAX];
} mg_mapping;


mg_mapping* mg_gamepad_find_valid_mapping(mg_gamepad* gamepad); 

#endif
