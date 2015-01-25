#include <iostream>

#include <SDL.h>
#include <SDL_image.h>

#include "GetTheFlag.h"
#include "Input.h"
#include "Level.h"

//#define RENDER_DEBUG

struct Player {
    Vec2 position;
    Vec2 velocity;
    Vec2 accel;
    Vec2 aimDir;
    real32 size;
    SDL_Surface* bitmaps[3];
};

#define MAX_BULLET_COUNT 50

struct Bullet {
    Vec2 position;
    Vec2 velocity;
};


struct Game {
    bool running;
    Player player;
    Level level;
};

// TODO: Free memory
SDL_Surface* loadBitmap(const char* filename)
{
    SDL_RWops *rwop;
    rwop = SDL_RWFromFile(filename, "rb");
    if(!rwop)
    {
        printf("SDL_RWFromFile: %s\n", IMG_GetError());
        
        ASSERT(false);
    }
    SDL_Surface* bitmap = IMG_LoadPNG_RW(rwop);
    if(!bitmap)
    {
        printf("IMG_LoadPNG_RW: %s\n", IMG_GetError());
        printf("Error loading: %s\n", filename);
        
        ASSERT(false);
    }
    return bitmap;
}

uint32 createBullet(uint32& bulletCount)
{
    ASSERT(bulletCount < MAX_BULLET_COUNT - 1);
    return bulletCount++;
}

void releaseBullet(Bullet* bullets, uint32& bulletCount,uint32 bulletIndex)
{
    bullets[bulletIndex] = bullets[--bulletCount];
}

inline uint32 levelUnitToPixel(real32 pixerPerUnit, real32 c)
{
    return roundReal32toInt32(c*pixerPerUnit);
}


// TODO: Fixed framerate
// TODO: Multiple Monitor
// TODO: Understand how flip and blit works with monitor refresh sync
// TODO: Understand the renderer thing
// TODO: Improve the timer (fixed dt)
// TODO: Make assert that prints a message
// TODO: Rotate sprites
// TODO: Collision Line/Rect?
// TODO: Fix collision bug at the lower right corner of the level


int main()
{
    
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) == 0)
    {
        uint32 ScreenWidth = 640;
        uint32 ScreenHeight = 480;
        
        SDL_Window* window = SDL_CreateWindow("Get The Flag", 0, 0, ScreenWidth, ScreenHeight, 0);
        SDL_Surface* window_surface = 0;
        int32 monitorRefreshRate = 60;
        int32 gameRefreshRate = monitorRefreshRate;
        real32 targetMsPerFrame = 1000.0f / gameRefreshRate;
        
        if(window)
        {
            window_surface = SDL_GetWindowSurface(window);
            ASSERT(window_surface);
           
            Level level;
            if(!loadLevel(&level))
            {
                printf("loadLevel: %s\n", IMG_GetError());
                ASSERT(false);
            }
            level.pixelPerUnit = ((real32)ScreenWidth)/((real32)level.width);
            
            Player player;
            // acceleration and drag in m/s;
            real32 drag = 20.0f;
            real32 acc = 200.0f;
            player.position = Vec2(5,10);
            player.velocity = Vec2(0,0);
            player.aimDir = Vec2(1,0);
            player.size = 3.0f;
            
            // Load player bitmap
            player.bitmaps[0] = loadBitmap("data/player1_right_standing.png");
            player.bitmaps[1] = loadBitmap("data/player1_right_walking_1.png");
            player.bitmaps[2] = loadBitmap("data/player1_right_walking_2.png");
            Vec2 playerSize(0.4*player.size, player.size);
            
            
            // Load background bitmap
            SDL_Surface* backgroundBitmap = loadBitmap("data/bg.png");
            if(!backgroundBitmap)
            {
                printf("loadBitmap: %s\n", IMG_GetError());
            }
            
            // Load Level element bitmaps
            SDL_Surface* gameEntitySurfaces[MAX_ENTITY_TYPE];
            const char* gameEntityImageFilename[MAX_ENTITY_TYPE];
            gameEntityImageFilename[TILE_TYPE::WALL] = "data/brick.png";
            gameEntityImageFilename[TILE_TYPE::SHOTGUN] = "data/shotgun.png";
            gameEntityImageFilename[TILE_TYPE::MACHINE_GUN] = "data/machine_gun.png";
            gameEntityImageFilename[TILE_TYPE::HEART] = "data/heart_full.png";
            gameEntityImageFilename[TILE_TYPE::BOMB] = "data/bomb.png";
            // Load all the images of the tile sprites
            for (uint32 i = 0; i < TILE_TYPE::MAX_ENTITY_TYPE; i++)
            {
                gameEntitySurfaces[i] = loadBitmap(gameEntityImageFilename[i]);
            }
            
            // Load Other bitmaps
            SDL_Surface* bulletBitmap = loadBitmap("data/bullet.png");
            
            // Pool for bullets
            Bullet bullets[MAX_BULLET_COUNT];
            uint32 bulletCount = 0;
            real32 bulletSpeed = 30.0f;
            Vec2 bulletSize(bulletBitmap->w/level.pixelPerUnit,
                            bulletBitmap->h/level.pixelPerUnit);
            
            Input input;
            memset(&input, 0, sizeof(input));
            
            // Timers
            uint64 lastCounter = SDL_GetPerformanceCounter();
            uint64 endCounter = 0;
            uint64 counterFrequency = SDL_GetPerformanceFrequency();
            
            bool running = true;
            
            // The main Loop
            while (running)
            {
                
                // Time delta in seconds
                real32 dt = targetMsPerFrame/1000.0f;
                
                processInput(&input);
                if(input.keyStates[QUIT].clicked
                   || input.keyStates[ESCAPE].clicked)
                {
                    running = false;
                }
                
                
                { // Player Update
                    // Fire
                    if(input.keyStates[FIRE1].clicked)
                    {
                        uint32 bulletIndex = createBullet(bulletCount);
                        bullets[bulletIndex].position = player.position;
                        bullets[bulletIndex].velocity = player.aimDir;
                    }
                    
                    player.accel = Vec2(0,0);
                    if (input.keyStates[UP].held)
                    {
                        player.accel.y = -1.0f;
                    }
                    if (input.keyStates[DOWN].held)
                    {
                        player.accel.y = 1.0f;
                    }
                    if (input.keyStates[LEFT].held)
                    {
                        player.accel.x = -1.0f;
                    }
                    if (input.keyStates[RIGHT].held)
                    {
                        player.accel.x = 1.0f;
                    }
                    if(abs(player.accel.x) > 0.0f || abs(player.accel.y) > 0.0f)
                    {
                        Vec2 n = normalize(player.accel);
                        ASSERT(!(n.x!=n.x));
                        player.aimDir = player.accel;
                    }
                    Vec2 acceleration = acc*player.accel - drag*player.velocity;
                    player.velocity += acceleration * dt;
                    Vec2 newPos =  player.position + player.velocity * dt;
                    
                    // Resolve collisions
                    // Stay inside the Level
                    newPos = max(newPos,Vec2(0,0));
                    newPos = min(newPos,Vec2(level.width,level.height));
                    player.position = newPos;
                    
                    Rect playerRec = {
                        newPos - 0.5f*playerSize,
                        newPos + 0.5F*playerSize
                    };
                    
                    Vec2 penetration;
                    if ( levelRectCollides(&level, playerRec, penetration) )
                    {
                        player.position -= penetration;
                    }
                }
                
                { // Update Bullets
                    // Move the bullet forward
                    real32 levelWidth = (real32)level.width;
                    real32 levelHeight = (real32)level.height;
                    Rect bulletRect;
                    
                    for (uint32 i = 0; i < bulletCount; i++)
                    {
                        bullets[i].position += (bulletSpeed * dt) * bullets[i].velocity;
                        
                        bulletRect.min = bullets[i].position - 0.5f*bulletSize;
                        bulletRect.max = bullets[i].position + 0.5f*bulletSize;
                        
                        if (isOutsideLevel(levelWidth, levelHeight, bullets[i].position)
                            || levelRectCollides(&level, bulletRect))
                        {
                            releaseBullet(bullets, bulletCount, i);
                        }
                    }
                }
                
                //Animate
                SDL_Surface* playerBitmap;
                if(player.velocity.x > 0 || player.velocity.y > 0)
                {
                    playerBitmap = player.bitmaps[1];
                }
                else
                {
                    playerBitmap = player.bitmaps[0];
                }
                
                // Draw
                {
                    // clear the window surface
                    SDL_FillRect(window_surface, 0, 0);
                    
                    // Background
                    SDL_BlitSurface(backgroundBitmap,0,window_surface,0);
                
                    real32 pixPerUnit = level.pixelPerUnit;
                    
                    // World elements
                    for (uint32 j = 0; j < level.height; j++)
                    {
                        for (uint32 i = 0; i < level.width; i++)
                        {
                            uint8 value = levelValueAtTile(&level,i,j);
                            if(value < TILE_TYPE::MAX_ENTITY_TYPE)
                            {
                                SDL_Rect rect;
                                rect.x = i*level.pixelPerUnit;
                                rect.y = j*level.pixelPerUnit;
                                rect.w = level.pixelPerUnit;
                                rect.h = level.pixelPerUnit;
                                SDL_BlitScaled(gameEntitySurfaces[value],0,window_surface,&rect);
                            }
                        }
                    }
                    
                    // Bullets
                    SDL_Rect bulletRect;
                    bulletRect.w = levelUnitToPixel(pixPerUnit, bulletSize.x);
                    bulletRect.h = levelUnitToPixel(pixPerUnit, bulletSize.y);
                    for (uint32 i = 0; i < bulletCount; i++)
                    {
                        bulletRect.x = levelUnitToPixel(pixPerUnit, bullets[i].position.x - 0.5f*bulletSize.x);
                        bulletRect.y = levelUnitToPixel(pixPerUnit, bullets[i].position.y - 0.5f*bulletSize.y);
                        SDL_BlitSurface(bulletBitmap,0,window_surface,&bulletRect);
                    }
                    
                    // Player
                    SDL_Rect playerRect;
                    playerRect.w = levelUnitToPixel(pixPerUnit, player.size);
                    playerRect.h = levelUnitToPixel(pixPerUnit, player.size);
                    playerRect.x = levelUnitToPixel(pixPerUnit, player.position.x-0.5f*player.size);
                    playerRect.y = levelUnitToPixel(pixPerUnit, player.position.y-0.5f*player.size);
                    
                    SDL_BlitScaled(playerBitmap,0,window_surface,&playerRect);
                }
      
#ifdef RENDER_DEBUG
                {
                    static SDL_Surface* debugSurface = 0;
                    if(!debugSurface)
                    {
                        debugSurface = SDL_CreateRGBSurface(0, 64, 64, 32,0,0,0,0);
                        memset(debugSurface->pixels, 255, sizeof(uint32)*64*64);
                    }
                    
                    // Collision rect
                    real32 pixPerUnit = level.pixelPerUnit;
                    SDL_Rect debugPlayerRect;
                    debugPlayerRect.w = levelUnitToPixel(pixPerUnit, playerSize.x);
                    debugPlayerRect.h = levelUnitToPixel(pixPerUnit, playerSize.y);
                    debugPlayerRect.x = levelUnitToPixel(pixPerUnit, player.position.x - 0.5f*playerSize.x);
                    debugPlayerRect.y = levelUnitToPixel(pixPerUnit, player.position.y - 0.5f*playerSize.y);

                    SDL_BlitScaled(debugSurface,0,window_surface,&debugPlayerRect);
                }
                
#endif
                
                // swap
                SDL_UpdateWindowSurface(window);
                
                // Update the clock
                endCounter = SDL_GetPerformanceCounter();
                
                uint64 couterElapsed = endCounter - lastCounter;
                real32 msElapsed = ((1000.0f * (real32)couterElapsed)/(real32)counterFrequency);
                
                if(msElapsed < targetMsPerFrame)
                {
                    SDL_Delay((uint32)targetMsPerFrame-msElapsed);
                    lastCounter = endCounter;
                    endCounter = SDL_GetPerformanceCounter();
                    couterElapsed = endCounter - lastCounter;
                    msElapsed  +=((1000.0f * (real32)couterElapsed)/(real32)counterFrequency);
                }
                else
                {
                    // TODO: LOG
                    std::cout << "LOOONNNGG FRAAAAMMMME!!" << std::endl;
                }
                
                //int32 fps = 1000/msElapsed;
                //printf("%f ms, %d fps %d\n",msElapsed,fps,bulletCount);
                lastCounter = endCounter;
            }
            // Cleanup
            SDL_DestroyWindow(window);
        }
        else
        {
            // TODO: LOG
        }
	}
    else
    {
        // TODO: LOG
    }
	
	SDL_Quit();
	return 0;
}
