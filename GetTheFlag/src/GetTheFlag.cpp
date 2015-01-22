#include <iostream>
#include <cassert>

#include <SDL.h>
#include <SDL_image.h>

#include "Input.h"
#define ASSERT assert

typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32 ;
typedef long int64 ;
typedef unsigned long uint64;

struct Game {
    bool running;
};

struct Player {
    int32 x;
    int32 y;
    int32 dx;
    int32 dy;
    
    SDL_Surface* bitmap;
};

enum TILE_TYPE {
    BLANK = 0,
    WALL,
    MACHINE_GUN,
    SHOTGUN,
    GRENADE,
    HEART
};


SDL_Surface* loadBitmap(const char* filename)
{
    SDL_RWops *rwop;
    rwop = SDL_RWFromFile(filename, "rb");
    if(!rwop)
    {
        printf("SDL_RWFromFile: %s\n", IMG_GetError());
        
    }
    SDL_Surface* bitmap = IMG_LoadPNG_RW(rwop);
    return bitmap;
}

struct Level {
    uint8* tiles;
    uint32 width;
    uint32 height;
};

inline uint8 levelValueAt(Level* level, uint32 x, uint32 y){
    return level->tiles[x+y*level->width];
}

bool loadLevel(Level* level)
{
    SDL_Surface* levelBitmap = loadBitmap("lvl1.png");
    if (levelBitmap)
    {
        uint32 LEVEL_WIDTH = levelBitmap->w;
        uint32 LEVEL_HEIGHT = levelBitmap->h;
        uint8* tiles = new uint8[LEVEL_WIDTH*LEVEL_WIDTH];
        memset(tiles, 0, LEVEL_HEIGHT*LEVEL_WIDTH*sizeof(uint8));
        for (uint32 j = 0; j < LEVEL_HEIGHT; j++)
        {
            for (uint32 i = 0; i < LEVEL_WIDTH; i++)
            {
                if(i==j)
                {
                    tiles[i+j*LEVEL_WIDTH] = TILE_TYPE::WALL;
                }
            }
        }
        level->tiles = tiles;
        level->width = LEVEL_WIDTH;
        level->height = LEVEL_HEIGHT;
        
        return true;
    }
    else
    {
        printf("loadBitmap: %s\n", IMG_GetError());
        return false;
    }
}

int main()
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
            uint32 width = 5;
            uint32 height = 5;
            
            Player player;
            int32 vel = 10;
            player.x = 0;
            player.y = 0;
            player.dx = 0;
            player.dy = 0;
            
            SDL_Rect playerRect = { 0, 0, (int32)width, (int32)height};
            
        
            SDL_Surface* playerBitmap = loadBitmap("data/player1_right_feet_left_gun_right.png");
            if(!playerBitmap)
            {
                printf("loadBitmap: %s\n", IMG_GetError());
            }
            SDL_Surface* backgroundBitmap = loadBitmap("data/bg.png");
            if(!backgroundBitmap)
            {
                printf("loadBitmap: %s\n", IMG_GetError());
            }
            SDL_Surface* wallBitmap = loadBitmap("data/brick.png");
            if(!wallBitmap)
            {
                printf("loadBitmap: %s\n", IMG_GetError());
            }
            
            Input input;
            memset(&input, 0, sizeof(input));
            
            
            Level level;
            if(!loadLevel(&level))
            {
                printf("loadLevel: %s\n", IMG_GetError());
            }
            
            bool running = true;
            // The main Loop
            while (running)
            {
                processInput(&input);
                
                if(input.quitKeyDown)
                {
                    running = false;
                }
                
                player.dx = 0;
                player.dy = 0;
                if (input.upKeyDown)
                {
                    player.dy = -vel;
                }
                if (input.downKeyDown)
                {
                    player.dy = vel;
                }
                if (input.leftKeyDown)
                {
                    player.dx = -vel;
                }
                if (input.rightKeyDown)
                {
                    player.dx = vel;
                }
                
                // Update
                player.x += player.dx;
                player.y += player.dy;
                
                // Draw
                SDL_RenderClear(renderer);
                // clear the window surface
                SDL_FillRect(window_surface, 0, 0);
                
                // Background
                SDL_BlitSurface(backgroundBitmap,0,window_surface,0);
            
                // World elements
                for (uint32 j = 0; j < level.height; j++)
                {
                    for (uint32 i = 0; i < level.width; i++)
                    {
                        if(levelValueAt(&level,i,j) == TILE_TYPE::WALL)
                        {
                            int w = wallBitmap->w;
                            int h = wallBitmap->h;
                            SDL_Rect rect {(int)(i*w),(int)(j*h),w,h};
                            SDL_BlitSurface(wallBitmap,0,window_surface,&rect);
                        }
                    }
                }
                
                // Player
                playerRect.x = player.x;
                playerRect.y = player.y;
                SDL_BlitSurface(playerBitmap,0,window_surface,&playerRect);
                
                // swap
                SDL_UpdateWindowSurface(window);
            }
            
            SDL_FreeSurface(playerBitmap);
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
