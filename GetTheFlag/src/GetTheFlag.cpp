#include <iostream>
#include <cassert>

#include <SDL.h>


#define ASSERT assert

typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32 ;
typedef long int64 ;
typedef unsigned long uint64;

int main(int argc, char** argv)
{
    
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) == 0)
    {
        SDL_Window* window = SDL_CreateWindow("Get The Flag", 0, 0, 640, 480, SDL_WINDOW_OPENGL);
        SDL_Renderer* renderer = 0;
        SDL_Surface* window_surface = 0;
        
        if(window)
        {
            window_surface = SDL_GetWindowSurface(window);
            ASSERT(window_surface);
            
            // Create dummy image
            uint32 rmask = 0x000000ff;
            uint32 gmask = 0x0000ff00;
            uint32 bmask = 0x00ff0000;
            uint32 amask = 0xff000000;
            
            uint32 width = 100;
            uint32 height = 100;
            SDL_Surface* gradient = SDL_CreateRGBSurface(0, width, height, 32, rmask, gmask, bmask, amask);
            ASSERT(gradient);
            uint32* pixels = (uint32*)gradient->pixels;
            uint32 yellow = 0xff00ffff;
            uint32 red = 0xff0000ff;
            for (int32 i = 0; i < gradient->w; i++) {
                for (int32 j = 0; j < gradient->h; j++) {
                    pixels[i+gradient->h*j] = red;
                }
            }
            
            SDL_BlitSurface(gradient,0,window_surface,0);
            
            bool running = true;
            // The main Loop
            while (running)
            {
                SDL_Event event;
                while(SDL_PollEvent(&event))
                {
                    switch (event.type)
                    {
                        case SDL_MOUSEMOTION:
                        {
                            
                        }break;
                            
                        case SDL_KEYDOWN:
                        {
                            if(event.key.keysym.sym == SDLK_ESCAPE)
                            {
                                running = false;
                            }
                            
                        }break;
                            
                        case SDL_KEYUP:
                        {
                            
                        }break;
                            
                        case SDL_QUIT:
                        {
                            running = false;
                        }
                            
                        default:
                            break;
                    }
                }
                
                
                SDL_UpdateWindowSurface(window);
            }
            
            SDL_FreeSurface(gradient);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
        }
        else
        {
            //TODO: LOG
        }
	}
    else
    {
        //TODO: LOG
    }
	
	SDL_Quit();
	return 0;
}
