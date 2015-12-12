
#include "Input.h"

// TODO: Cleanup the if if if if
void processInput(Input* input)
{
    for (uint32 i = 0; i < KEYS::NUM_KEYS; i++)
    {
        input->keyStates[i].clicked = false;
    }
    
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        if(event.key.repeat == 0)
        {
            switch (event.type)
            {
                case SDL_MOUSEMOTION:
                {
                    input->mouseX  = event.motion.x;
                    input->mouseY  = event.motion.y;
                }break;
                    
                case SDL_KEYUP:
                {
                    if(event.key.keysym.sym == SDLK_UP)
                    {
                        input->keyStates[UP].held = false;
                    }
                    if(event.key.keysym.sym == SDLK_DOWN)
                    {
                        input->keyStates[DOWN].held = false;
                    }
                    if(event.key.keysym.sym == SDLK_LEFT)
                    {
                        input->keyStates[LEFT].held = false;
                    }
                    if(event.key.keysym.sym == SDLK_RIGHT)
                    {
                        input->keyStates[RIGHT].held = false;
                    }
                    if(event.key.keysym.sym == SDLK_r)
                    {
                        input->keyStates[DEBUG_RELOAD_SHADERS].held = false;
                    }
                    if(event.key.keysym.sym == SDLK_g)
                    {
                        input->keyStates[DEBUG_SHOW_DISTANCE_FIELD].held = false;
                    }
                    if(event.key.keysym.sym == SDLK_i)
                    {
                        input->keyStates[DEBUG_SHOW_REGIONS].held = false;
                    }
                }break;
                    
                case SDL_KEYDOWN:
                {
                    if(event.key.keysym.sym == SDLK_UP)
                    {
                        input->keyStates[UP].clicked = true;
                        input->keyStates[UP].held = true;
                    }
                    if(event.key.keysym.sym == SDLK_DOWN)
                    {
                        input->keyStates[DOWN].clicked = true;
                        input->keyStates[DOWN].held = true;
                    }
                    if(event.key.keysym.sym == SDLK_LEFT)
                    {
                        input->keyStates[LEFT].clicked = true;
                        input->keyStates[LEFT].held = true;
                    }
                    if(event.key.keysym.sym == SDLK_RIGHT)
                    {
                        input->keyStates[RIGHT].clicked = true;
                        input->keyStates[RIGHT].held = true;
                    }
                    if(event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        input->keyStates[ESCAPE].clicked = true;
                        input->keyStates[ESCAPE].held = true;
                    }
                    if(event.key.keysym.sym == SDLK_SPACE)
                    {
                        input->keyStates[FIRE1].clicked = true;
                        input->keyStates[FIRE1].held = true;
                    }
                    if(event.key.keysym.sym == SDLK_b)
                    {
                        input->keyStates[FIRE2].clicked = true;
                        input->keyStates[FIRE2].held = true;
                    }
                    if(event.key.keysym.sym == SDLK_r)
                    {
                        input->keyStates[DEBUG_RELOAD_SHADERS].clicked = true;
                        input->keyStates[DEBUG_RELOAD_SHADERS].held = true;
                    }
                    if(event.key.keysym.sym == SDLK_d)
                    {
                        input->keyStates[DEBUG_SHOW_DISTANCE_FIELD].clicked = true;
                        input->keyStates[DEBUG_SHOW_DISTANCE_FIELD].held = true;
                    }
                    if(event.key.keysym.sym == SDLK_i)
                    {
                        input->keyStates[DEBUG_SHOW_REGIONS].clicked = true;
                        input->keyStates[DEBUG_SHOW_REGIONS].held = true;
                    }
                }break;
                    
                case SDL_QUIT:
                {
                    input->keyStates[QUIT].clicked = true;
                }
                    
                default:
                {
                }break;
            }
        }
    }
}
