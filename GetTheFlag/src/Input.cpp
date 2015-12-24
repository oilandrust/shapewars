
#include "Input.h"

void processKeyEvent(Input* input, SDL_Event* event, KEY key) {
    if(event->type == SDL_KEYDOWN) {
        input->keyStates[key].clicked = true;
        input->keyStates[key].held = true;
    } else if (event->type == SDL_KEYUP) {
        input->keyStates[key].held = false;
    }
}

void processInput(Input* input)
{
    for (uint32 i = 0; i < KEY::NUM_KEYS; i++)
    {
        input->keyStates[i].clicked = false;
    }
    
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        if(event.key.repeat == 0)
        {
            if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                if(event.key.keysym.sym == SDLK_UP)
                {
                    processKeyEvent(input, &event, UP);
                }
                else if(event.key.keysym.sym == SDLK_DOWN)
                {
                    processKeyEvent(input, &event, DOWN);
                }
                else if(event.key.keysym.sym == SDLK_LEFT)
                {
                    processKeyEvent(input, &event, LEFT);
                }
                else if(event.key.keysym.sym == SDLK_RIGHT)
                {
                    processKeyEvent(input, &event, RIGHT);
                }
                else if(event.key.keysym.sym == SDLK_ESCAPE) {
                    processKeyEvent(input, &event, ESCAPE);
                }
                else if(event.key.keysym.sym == SDLK_SPACE) {
                    processKeyEvent(input, &event, FIRE1);
                }
                else if(event.key.keysym.sym == SDLK_b) {
                    processKeyEvent(input, &event, FIRE1);
                }
                else if(event.key.keysym.sym == SDLK_r)
                {
                    processKeyEvent(input, &event, DEBUG_RELOAD_SHADERS);
                }
                else if(event.key.keysym.sym == SDLK_f)
                {
                    processKeyEvent(input, &event, DEBUG_SHOW_DISTANCE_FIELD);
                }
                else if(event.key.keysym.sym == SDLK_i)
                {
                    processKeyEvent(input, &event, DEBUG_SHOW_REGIONS);
                }
                else if(event.key.keysym.sym == SDLK_c)
                {
                    processKeyEvent(input, &event, DEBUG_SHOW_CONTOURS);
                }
                else if(event.key.keysym.sym == SDLK_t)
                {
                    processKeyEvent(input, &event, DEBUG_SHOW_TRI_REGIONS);
                }
                else if(event.key.keysym.sym == SDLK_n)
                {
                    processKeyEvent(input, &event, DEBUG_SHOW_POLY_REGIONS);
                }
                else if(event.key.keysym.sym == SDLK_d)
                {
                    processKeyEvent(input, &event, DEBUG_SHOW_DUAL_MESH);
                }
            }
            switch (event.type)
            {
                case SDL_MOUSEMOTION:
                {
                    input->mouseX  = event.motion.x;
                    input->mouseY  = event.motion.y;
                }break;
                    
                case SDL_QUIT:
                {
                    input->keyStates[QUIT].clicked = true;
                } break;
                    
                default:
                {
                }break;
            }
        }
    }
}
