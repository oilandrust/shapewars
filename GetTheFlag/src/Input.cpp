
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
