#define RGFW_IMPLEMENTATION
#include "deps/RGFW.h"

#define RSGL_INT_DEFINED
#define RSGL_IMPLEMENTATION
#include "deps/RSGL.h"
#include "deps/RSGL_gl.h"

#define TG_IMPLEMENTATION
#include "tinygamepad.h"


int main(void) {
    RGFW_window* win = RGFW_createWindow("name", (RGFW_rect){500, 500, 800, 480}, RGFW_windowCenter);

	RSGL_init(RSGL_AREA(win->r.w, win->r.h), RGFW_getProcAddress, RSGL_GL_renderer());
                     
    RSGL_font* font = NULL;//RSGL_loadFont("DejaVuSans.ttf");
    if (font != NULL)
        RSGL_setFont(font);
  
    tg_gamepads gamepads = {0};
    tg_gamepads_init(&gamepads);

    tg_gamepad* gamepad = gamepads.list.head;
    char log[256];

    while (RGFW_window_shouldClose(win) == false) {
        while (RGFW_window_checkEvent(win)) {
            if (win->event.type != RGFW_keyPressed)
                continue;
           
            if (RGFW_isPressed(win, RGFW_left) && gamepad->prev != NULL) {
                gamepad = gamepad->prev;
            }

            if (RGFW_isPressed(win, RGFW_right) && gamepad->next != NULL) {
                gamepad = gamepad->next;
            }
        }

        tg_event ev;

        while (tg_gamepads_update(&gamepads, &ev)) {
            switch (ev.type) {
                case TG_EVENT_GAMEPAD_CONNECT:
                    snprintf(log, sizeof(log), "new gamepad connected");
                    break;
                case TG_EVENT_GAMEPAD_DISCONNECT:
                    snprintf(log, sizeof(log), "gamepad disconnected");
                    break;
                case TG_EVENT_BUTTON_PRESS:
                    snprintf(log, sizeof(log), "button pressed %i", ev.button);
                    break;
                case TG_EVENT_BUTTON_RELEASE:
                    snprintf(log, sizeof(log), "button released %i", ev.button);
                    break;
                case TG_EVENT_AXIS_MOVE:
                    snprintf(log, sizeof(log), "axis moved %i", ev.axis);
                    break;
                default: break;
            }        

        }

        if (gamepad == NULL) {
            gamepad = gamepads.list.head;
        }


        const RSGL_pointF pointLeft = RSGL_POINTF(((float)gamepad->axes[TG_AXIS_LEFT_X].value / 100.0f) * 30, 
                                                  ((float)gamepad->axes[TG_AXIS_LEFT_Y].value / 100.0f) * 30);

        const RSGL_pointF pointRight = RSGL_POINTF(((float)gamepad->axes[TG_AXIS_RIGHT_X].value / 100.0f) * 30, 
                                                   ((float)gamepad->axes[TG_AXIS_RIGHT_Y].value / 100.0f) * 30);

        const RSGL_pointF trigger = RSGL_POINTF(((((gamepad->axes[TG_AXIS_LEFT_TRIGGER].value) + 100)) / 200.0f) * 100, 
                                                ((((gamepad->axes[TG_AXIS_RIGHT_TRIGGER].value) + 100)) / 200.0f) * 100);


        RSGL_clear(RSGL_RGB(200, 200, 200));

        RSGL_drawText(log, RSGL_CIRCLE(0, 450, 25), RSGL_RGB(255, 0, 0));
        if (gamepad != NULL) {
            RSGL_drawText(gamepad->name, RSGL_CIRCLE(400, 450, 25), RSGL_RGB(255, 0, 0));
            
            const RSGL_rect gamepadRect = RSGL_RECT(win->r.w * (1.0f / 8.0f), (win->r.h * (1.0f / 8.0f)),  
                                                    win->r.w - (win->r.w * (2.0f / 8.0f)), win->r.h - (win->r.h * (2.0f / 8.0f)));
            
            RSGL_drawRoundRect(gamepadRect, RSGL_POINT(20, 20), RSGL_RGB(20, 20, 20));
          
            #define BUTTON_COLOR(i, r, g, b) (gamepad->buttons[i].current) ? RSGL_RGB(r, g, b) : RSGL_RGB(100, 100, 100)

            int i;
            for (i = 0; i < TG_BUTTON_COUNT; i++) {
                if (gamepad->buttons[i].supported == false)
                    continue;

                switch (i) {
                        case TG_BUTTON_DPAD_UP:
                            RSGL_drawRect(RSGL_RECT(gamepadRect.x + (gamepadRect.w * (1.0f / 8.0f)), 
                                                gamepadRect.y + (gamepadRect.h * (2.0f / 8.0f)), 20, 20), RSGL_RGB(100, 100, 100));

                            RSGL_drawCircle(RSGL_CIRCLE(gamepadRect.x + (gamepadRect.w * (1.0f / 8.0f)) - 40, 
                                                        gamepadRect.y + (gamepadRect.h * (2.0f / 8.0f)) - 40, 100), RSGL_RGB(50, 50, 50));

                            RSGL_drawRect(RSGL_RECT(gamepadRect.x + (gamepadRect.w * (1.0f / 8.0f)), 
                                                    gamepadRect.y + (gamepadRect.h * (2.0f / 8.0f)) - 40, 20, 40), BUTTON_COLOR(i, 200, 0, 0));
                            break;
                        case TG_BUTTON_DPAD_LEFT:
                            RSGL_drawRect(RSGL_RECT(gamepadRect.x + (gamepadRect.w * (1.0f / 8.0f)) - 40, 
                                                    gamepadRect.y + (gamepadRect.h * (2.0f / 8.0f)), 40, 20), BUTTON_COLOR(i, 200, 0, 0));
                            break;
                        case TG_BUTTON_DPAD_RIGHT:
                            RSGL_drawRect(RSGL_RECT(gamepadRect.x + (gamepadRect.w * (1.0f / 8.0f)) + 20, 
                                                    gamepadRect.y + (gamepadRect.h * (2.0f / 8.0f)), 40, 20), BUTTON_COLOR(i, 200, 0, 0));
                            break;
                        case TG_BUTTON_DPAD_DOWN:
                            RSGL_drawRect(RSGL_RECT(gamepadRect.x + (gamepadRect.w * (1.0f / 8.0f)), 
                                                gamepadRect.y + (gamepadRect.h * (2.0f / 8.0f)) + 20, 20, 40), BUTTON_COLOR(i, 200, 0, 0));
                            break;
                        case TG_BUTTON_NORTH:
                            RSGL_drawCircle(RSGL_CIRCLE(gamepadRect.x + (gamepadRect.w * (6.0f / 8.0f)), gamepadRect.y + (gamepadRect.h * (1.0f / 8.0f)), 30),
                                            BUTTON_COLOR(i, 200, 200, 0));
                            break;
 
                        case TG_BUTTON_SOUTH:
                            RSGL_drawCircle(RSGL_CIRCLE(gamepadRect.x + (gamepadRect.w * (6.0f / 8.0f)), gamepadRect.y + (gamepadRect.h * (2.5f / 8.0f)), 30),
                                            BUTTON_COLOR(i, 0, 200, 0));
                            break;

                        case TG_BUTTON_WEST:
                            RSGL_drawCircle(RSGL_CIRCLE(gamepadRect.x + (gamepadRect.w * (5.5f / 8.0f)), gamepadRect.y + (gamepadRect.h * (1.75f / 8.0f)), 30),
                                            BUTTON_COLOR(i, 0, 0, 200));
                            break;
 
                        case TG_BUTTON_EAST:
                            RSGL_drawCircle(RSGL_CIRCLE(gamepadRect.x + (gamepadRect.w * (6.5f / 8.0f)), gamepadRect.y + (gamepadRect.h * (1.75f / 8.0f)), 30),
                                            BUTTON_COLOR(i, 200, 0, 0));
                            break;
                        case TG_BUTTON_GUIDE:
                            RSGL_drawCircle(RSGL_CIRCLE(gamepadRect.x + (gamepadRect.w * (3.5 / 8.0f)), gamepadRect.y + (gamepadRect.h * (1.0 / 8.0f)), 50),
                                            BUTTON_COLOR(i, 200, 0, 0));
                            break;
                        case TG_BUTTON_BACK:
                            RSGL_drawCircle(RSGL_CIRCLE(gamepadRect.x + (gamepadRect.w * (2.5 / 8.0f)), gamepadRect.y + (gamepadRect.h * (2.0 / 8.0f)), 25),
                                            BUTTON_COLOR(i, 200, 0, 0));
                            break;
                        case TG_BUTTON_START:
                            RSGL_drawCircle(RSGL_CIRCLE(gamepadRect.x + (gamepadRect.w * (4.5 / 8.0f)), gamepadRect.y + (gamepadRect.h * (2.0 / 8.0f)), 25),
                                            BUTTON_COLOR(i, 200, 0, 0));
                            break;
                        case TG_BUTTON_LEFT_SHOULDER:
                             RSGL_drawRect(RSGL_RECT(gamepadRect.x + 20, gamepadRect.y - 20, 60, 20), BUTTON_COLOR(i, 200, 0, 0));
                             break;
                        case TG_BUTTON_RIGHT_SHOULDER:
                             RSGL_drawRect(RSGL_RECT(gamepadRect.x + gamepadRect.w - 80, gamepadRect.y - 20, 60, 20), BUTTON_COLOR(i, 200, 0, 0));
                            break;
                        case TG_BUTTON_LEFT_TRIGGER:
                             RSGL_drawRect(RSGL_RECT(gamepadRect.x + 30, gamepadRect.y - 40, 40, 20), BUTTON_COLOR(i, 200, 0, 0));
                             break;
                        case TG_BUTTON_RIGHT_TRIGGER:
                             RSGL_drawRect(RSGL_RECT(gamepadRect.x + gamepadRect.w - 70, gamepadRect.y - 40, 40, 20), BUTTON_COLOR(i, 200, 0, 0));
                            break; 
                        default: break;
                }
            }

            for (i = 0; i < TG_AXIS_COUNT; i++) {
                if (gamepad->axes[i].supported == false)
                    continue;

                switch (i) {
                    case TG_AXIS_LEFT_X:
                        RSGL_drawCircle(RSGL_CIRCLE(gamepadRect.x + (gamepadRect.w * (2.0f / 8.0f)), gamepadRect.y + gamepadRect.h - 100, 80), RSGL_RGB(100, 100, 100));

                        if (gamepad->buttons[TG_BUTTON_LEFT_STICK].current == 0)
                            RSGL_drawCircle(RSGL_CIRCLE(gamepadRect.x + (gamepadRect.w * (2.15f / 8.0f)) + pointLeft.x, gamepadRect.y + gamepadRect.h - 90 + pointLeft.y, 60), RSGL_RGB(50, 50, 50));
                        else 
                            RSGL_drawCircle(RSGL_CIRCLE(gamepadRect.x + (gamepadRect.w * (2.15f / 8.0f)) + pointLeft.x, gamepadRect.y + gamepadRect.h - 90 + pointLeft.y, 60), RSGL_RGB(200, 0, 0));
                        i++;
                        break;
                    case TG_AXIS_RIGHT_X: 
                        RSGL_drawCircle(RSGL_CIRCLE(gamepadRect.x + (gamepadRect.w * (5.0f / 8.0f)), gamepadRect.y + gamepadRect.h - 100, 80), RSGL_RGB(100, 100, 100));
                        
                        if (gamepad->buttons[TG_BUTTON_RIGHT_STICK].current == 0) 
                            RSGL_drawCircle(RSGL_CIRCLE(gamepadRect.x + (gamepadRect.w * (5.15f / 8.0f)) + pointRight.x , gamepadRect.y + gamepadRect.h - 90 + pointRight.y, 60),  RSGL_RGB(50, 50, 50));
                        else
                            RSGL_drawCircle(RSGL_CIRCLE(gamepadRect.x + (gamepadRect.w * (5.15f / 8.0f)) + pointRight.x , gamepadRect.y + gamepadRect.h - 90 + pointRight.y, 60), RSGL_RGB(200, 0, 0)); 
                        i++;
                        break;
                    case TG_AXIS_LEFT_TRIGGER:
                        RSGL_drawRect(RSGL_RECT(gamepadRect.x - 40, gamepadRect.y - 40, 20, 100), BUTTON_COLOR(TG_BUTTON_LEFT_TRIGGER, 200, 0, 0));
                            RSGL_drawRect(RSGL_RECT(gamepadRect.x - 40, gamepadRect.y - 40, 20, trigger.x), RSGL_RGB(200, 0, 0));
                        break;
                    case TG_AXIS_RIGHT_TRIGGER:
                        RSGL_drawRect(RSGL_RECT(gamepadRect.x + gamepadRect.w + 20, gamepadRect.y - 40, 20, 100), BUTTON_COLOR(TG_BUTTON_RIGHT_TRIGGER, 200, 0, 0));
                            RSGL_drawRect(RSGL_RECT(gamepadRect.x + gamepadRect.w + 20, gamepadRect.y - 40, 20, trigger.y), RSGL_RGB(200, 0, 0));
                        break;
                    default: break;
                }
            }
        }

        RSGL_draw();
		RGFW_window_swapBuffers(win);
    }
    
    if (font != NULL) {
        RSGL_freeFont(font);
    }

    tg_gamepads_free(&gamepads);

	RSGL_free();
	RGFW_window_close(win);
}
