#include <iostream>
#include <SDL.h>

#define int8  char;
#define uint8 unsigned char;
#define int16 short;
#define uint16 unsigned short;
#define int32 int;
#define uint32 unsigned int;
#define int64 long;
#define uint64 unsigned long;

int main(int argc, char** argv)
{
    
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) == 0)
    {
        SDL_Window* window = SDL_CreateWindow("Get The Flag", 0, 0, 640, 480, SDL_WINDOW_OPENGL);
        if(window)
        {
            bool running = true;
            // The main Loop
            while (running)
            {
                SDL_Event event;
                if(SDL_PollEvent(&event))
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
                            
                        default:
                            break;
                    }
                }
            }
            
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
