
#include "Input.h"

void processKeyEvent(Input* input, SDL_Event* event, KEY key)
{
    if (event->type == SDL_KEYDOWN) {
        input->keyStates[key].clicked = true;
        input->keyStates[key].held = true;
    }
    else if (event->type == SDL_KEYUP) {
        input->keyStates[key].held = false;
    }
}

void processInput(Input* input)
{
    for (uint32 i = 0; i < KEY::NUM_KEYS; i++) {
        input->keyStates[i].clicked = false;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {

        switch (event.type) {
        case SDL_MOUSEMOTION: {
            input->mouseX = event.motion.x;
            input->mouseY = event.motion.y;
            input->mousedX = event.motion.xrel;
            input->mousedY = event.motion.yrel;
        } break;

        case SDL_MOUSEBUTTONDOWN: {
            if (event.button.button == SDL_BUTTON_LEFT) {
                input->keyStates[MOUSE_LEFT].clicked = true;
                input->keyStates[MOUSE_LEFT].held = true;
            }
            else if (event.button.button == SDL_BUTTON_RIGHT) {
                input->keyStates[MOUSE_RIGHT].clicked = true;
                input->keyStates[MOUSE_RIGHT].held = true;
            }
        } break;

        case SDL_MOUSEBUTTONUP: {
            if (event.button.button == SDL_BUTTON_LEFT) {
                input->keyStates[MOUSE_LEFT].held = false;
            }
            else if (event.button.button == SDL_BUTTON_RIGHT) {
                input->keyStates[MOUSE_RIGHT].held = false;
            }
        } break;

        case SDL_QUIT: {
            input->keyStates[QUIT].clicked = true;
        } break;

        default: {
        } break;
        }

        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            if (event.key.keysym.sym == SDLK_UP) {
                processKeyEvent(input, &event, UP);
            }
            else if (event.key.keysym.sym == SDLK_DOWN) {
                processKeyEvent(input, &event, DOWN);
            }
            else if (event.key.keysym.sym == SDLK_LEFT) {
                processKeyEvent(input, &event, LEFT);
            }
            else if (event.key.keysym.sym == SDLK_RIGHT) {
                processKeyEvent(input, &event, RIGHT);
            }
            else if (event.key.keysym.sym == SDLK_ESCAPE) {
                processKeyEvent(input, &event, ESCAPE);
            }
            else if (event.key.keysym.sym == SDLK_SPACE) {
                processKeyEvent(input, &event, FIRE1);
            }
            else if (event.key.keysym.sym == SDLK_b) {
                processKeyEvent(input, &event, FIRE1);
            }
            else if (event.key.keysym.sym == SDLK_r) {
                processKeyEvent(input, &event, DEBUG_RELOAD_SHADERS);
            }
            else if (event.key.keysym.sym == SDLK_s) {
                processKeyEvent(input, &event, DEBUG_TOGGLE_FULLSCREEN);
            }
            else if (event.key.keysym.sym == SDLK_h) {
                processKeyEvent(input, &event, DEBUG_SHOW_MENU);
			}

        }
    }
}
