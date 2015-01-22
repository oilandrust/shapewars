
#include "Input.h"

void processInput(Input* input)
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
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
                    input->upKeyDown = false;
                }
                if(event.key.keysym.sym == SDLK_DOWN)
                {
                    input->downKeyDown = false;
                }
                if(event.key.keysym.sym == SDLK_LEFT)
                {
                    input->leftKeyDown = false;
                }
                if(event.key.keysym.sym == SDLK_RIGHT)
                {
                    input->rightKeyDown = false;
                }
                
            }break;
                
            case SDL_KEYDOWN:
            {
                if(event.key.keysym.sym == SDLK_UP)
                {
                    input->upKeyDown = true;
                }
                if(event.key.keysym.sym == SDLK_DOWN)
                {
                    input->downKeyDown = true;
                }
                if(event.key.keysym.sym == SDLK_LEFT)
                {
                    input->leftKeyDown = true;
                }
                if(event.key.keysym.sym == SDLK_RIGHT)
                {
                    input->rightKeyDown = true;
                }
                
                if(event.key.keysym.sym == SDLK_ESCAPE)
                {
                    input->quitKeyDown = true;
                }
                
            }break;
                
            case SDL_QUIT:
            {
                input->quitKeyDown = true;
            }
                
            default:
            {
            }break;
        }
    }
}
