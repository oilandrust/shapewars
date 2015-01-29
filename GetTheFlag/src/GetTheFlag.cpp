#include <cstdio>

#include <SDL.h>
#include <SDL_image.h>
#include <GL/glew.h>
#include <SDL_opengl.h>

#include "GetTheFlag.h"
#include "Input.h"
#include "Level.h"
#include "OpenGLClient.h"

//#define RENDER_DEBUG
#define RENDER_GL

struct Player {
    Vec2 position;
    Vec2 velocity;
    Vec2 accel;
    Vec2 aimDir;
    real32 spriteSize;
    real32 size;
    SDL_Surface* bitmaps[3];
    Texture textures[3];
};

#define MAX_BULLET_COUNT 50

struct Bullet {
    Vec2 position;
    Vec2 velocity;
};

uint32 createBullet(uint32& bulletCount)
{
    ASSERT(bulletCount < MAX_BULLET_COUNT - 1, "Too many bullets");
    return bulletCount++;
}

void releaseBullet(Bullet* bullets, uint32& bulletCount,uint32 bulletIndex)
{
    bullets[bulletIndex] = bullets[--bulletCount];
}

// TODO: Free memory
SDL_Surface* loadBitmap(const char* filename)
{
    SDL_RWops *rwop;
    rwop = SDL_RWFromFile(filename, "rb");
    if(!rwop)
    {
        printf("SDL_RWFromFile: %s\n", IMG_GetError());
        ASSERT(false, IMG_GetError());
    }
    SDL_Surface* bitmap = IMG_LoadPNG_RW(rwop);
    if(!bitmap)
    {
        printf("IMG_LoadPNG_RW: %s\n", IMG_GetError());
        printf("Error loading: %s\n", filename);
        ASSERT(false, IMG_GetError());
    }
    return bitmap;
}

bool loadTexture(Texture* tex, SDL_Surface* surface)
{
    tex->data = surface->pixels;
    tex->width = surface->w;
    tex->height = surface->h;
    return createTexture(tex);
}


inline uint32 levelUnitToPixel(real32 pixerPerUnit, real32 c)
{
    return roundReal32toInt32(c*pixerPerUnit);
}


// TODO: Shader level 140
// TODO: Multiple Monitor
// TODO: Understand how flip and blit works with monitor refresh sync
// TODO: Improve the timer (fixed dt)
// TODO: Rotate sprites
// TODO: Create shader, VBuffer

int main()
{
    
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
    {
        uint32 ScreenWidth = 640;
        uint32 ScreenHeight = 480;
        
        
        
        SDL_Window* window = SDL_CreateWindow("Get The Flag", 0, 0, ScreenWidth, ScreenHeight, SDL_WINDOW_OPENGL);
        SDL_Surface* window_surface = 0;
        
        int32 monitorRefreshRate = 30;
        int32 gameRefreshRate = monitorRefreshRate;
        real32 targetMsPerFrame = 1000.0f / gameRefreshRate;
        
        if(window)
        {
            window_surface = SDL_GetWindowSurface(window);
            ASSERT(window_surface, SDL_GetError());
            
            // Initialze OpenGL 3.1
            SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
            SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
            SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
           
            //Create context
            SDL_GLContext glContext = SDL_GL_CreateContext(window);
            ASSERT(!glGetError(), "Error Creating Contex");
            ASSERT(glContext, "OpenGL context could not be created! SDL Error: %s\n" + std::string(SDL_GetError()));
            
            //Initialize GLEW
            glewExperimental = GL_TRUE;
            GLenum result = glewInit();
            ASSERT(result == GLEW_OK, "Error initializing GLEW!" + std::string((const char*)glewGetErrorString(result)));
            
            //Use Vsync
            if( SDL_GL_SetSwapInterval( 1 ) < 0 )
            {
                printf( "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError() );
            }
            
            // Initialize the rendering resources
            Shader spriteShader;
            bool success = createShaderProgram(&spriteShader,"shaders/sprite.vs","shaders/sprite.fs");
            ASSERT(success, "Loading shader failed");
            spriteShader.projLoc = glGetUniformLocation(spriteShader.progId, "projection");
            spriteShader.posLoc = glGetUniformLocation(spriteShader.progId, "entity_position");
            spriteShader.sizeLoc = glGetUniformLocation(spriteShader.progId, "entity_size");
            spriteShader.texLoc = glGetUniformLocation(spriteShader.progId, "sprite_texture");
            
            Mesh spriteMesh;
            // 2 triangles -> 6 vertices + 6 tcs
            Vec2 rectVertices[12];
            //uint32 rectIndices[12] = {0,1,2,0,2,3};
            rectVertices[0] = Vec2(-.5f,-.5f);
            rectVertices[1] = Vec2(-.5f,.5f);
            rectVertices[2] = Vec2(.5f,.5f);
            rectVertices[3] = Vec2(-.5f,-.5f);
            rectVertices[4] = Vec2(.5f,.5f);
            rectVertices[5] = Vec2(.5f,-.5f);
            
            rectVertices[6] = Vec2(.0f,0.f);
            rectVertices[7] = Vec2(0.f,1.f);
            rectVertices[8] = Vec2(1.f,1.f);
            rectVertices[9] = Vec2(0.f,0.f);
            rectVertices[10] = Vec2(1.f,1.f);
            rectVertices[11] = Vec2(1.f,0.f);
            
            spriteMesh.positions = rectVertices;
            //spriteMesh.indices = rectIndices;
            createVertexAndIndexBuffer(&spriteMesh);
            
            // Load The Level
            Level level;
            if(!loadLevel(&level,"data/lvl2.bmp"))
            {
                printf("loadLevel: %s\n", IMG_GetError());
                ASSERT(false, IMG_GetError());
            }
            level.pixelPerUnit = ((real32)ScreenWidth)/((real32)level.width);
            
            Player player;
            // acceleration and drag in m/s;
            real32 drag = 20.0f;
            real32 acc = 200.0f;
            player.position = Vec2(3,3);
            player.velocity = Vec2(0,0);
            player.aimDir = Vec2(1,0);
            player.size = 3.0f;
            
            // Load player bitmap
            player.bitmaps[0] = loadBitmap("data/player1_right_standing.png");
            loadTexture(&player.textures[0], player.bitmaps[0]);
            player.bitmaps[1] = loadBitmap("data/player1_right_walking_1.png");
            loadTexture(&player.textures[1], player.bitmaps[1]);
            player.bitmaps[2] = loadBitmap("data/player1_right_walking_2.png");
            loadTexture(&player.textures[2], player.bitmaps[2]);
            Vec2 playerSize(0.4*player.size, player.size);
            
            
            // Load background bitmap
            SDL_Surface* backgroundBitmap = loadBitmap("data/bg.png");
            if(!backgroundBitmap)
            {
                printf("loadBitmap: %s\n", IMG_GetError());
            }
            Texture groungTexture;
            loadTexture(&groungTexture, backgroundBitmap);
            ASSERT(success, "Initializing texture failed");
            
            // Load Level element bitmaps
            SDL_Surface* gameEntitySurfaces[MAX_ENTITY_TYPE];
            const char* gameEntityImageFilename[MAX_ENTITY_TYPE];
            Texture gameEntityTextures[MAX_ENTITY_TYPE];
            gameEntityImageFilename[TILE_TYPE::WALL] = "data/brick.png";
            gameEntityImageFilename[TILE_TYPE::SHOTGUN] = "data/shotgun.png";
            gameEntityImageFilename[TILE_TYPE::MACHINE_GUN] = "data/machine_gun.png";
            gameEntityImageFilename[TILE_TYPE::HEART] = "data/heart_full.png";
            gameEntityImageFilename[TILE_TYPE::BOMB] = "data/bomb.png";
            // Load all the images of the tile sprites
            for (uint32 i = 0; i < TILE_TYPE::MAX_ENTITY_TYPE; i++)
            {
                gameEntitySurfaces[i] = loadBitmap(gameEntityImageFilename[i]);
                loadTexture(&gameEntityTextures[i], gameEntitySurfaces[i]);
            }
            
            // Load Other bitmaps
            SDL_Surface* bulletBitmap = loadBitmap("data/bullet.png");
            Texture bulletTexture;
            loadTexture(&bulletTexture, bulletBitmap);
            
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
            
            // Check error at initialization
            logOpenGLErrors();
            
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
                
                if(input.keyStates[RELOAD].clicked)
                {
                    glDeleteShader(spriteShader.progId);
                    createShaderProgram(&spriteShader,"shaders/sprite.vs","shaders/sprite.fs");
                    spriteShader.projLoc = glGetUniformLocation(spriteShader.progId, "projection");
                    spriteShader.posLoc = glGetUniformLocation(spriteShader.progId, "entity_position");
                    spriteShader.sizeLoc = glGetUniformLocation(spriteShader.progId, "entity_size");
                    spriteShader.texLoc = glGetUniformLocation(spriteShader.progId, "sprite_texture");
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
                        player.accel.y = 1.0f;
                    }
                    if (input.keyStates[DOWN].held)
                    {
                        player.accel.y = -1.0f;
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
                        player.aimDir = player.accel;
                    }
                    Vec2 acceleration = acc*player.accel - drag*player.velocity;
                    player.velocity += acceleration * dt;
                    
                    if( abs(player.velocity.x) > 0.0f || abs(player.velocity.y) > 0.0f)
                    {
                        Vec2 newPos = player.position + player.velocity * dt;;
                        
                        // Resolve collisions
                        Vec2 collisionPoint;
                        Vec2 normal;
                        if( levelRectCollides(&level,
                                              playerSize, player.position, player.velocity, dt,
                                              collisionPoint,normal ) )
                        {
                            // if we collide, we run the remaining distance along the wall
                            real32 dtr;
                            do
                            {
                                Vec2 dispInside = newPos - collisionPoint;
                                Vec2 dispCorrected = dispInside-dot(normal,dispInside)*normal;
                                
                                if(length(dispCorrected) > 0)
                                {
                                    real32 oldVelocity = length(player.velocity);
                                    dtr = dt - length(player.position - collisionPoint)*oldVelocity;
                                    player.velocity = oldVelocity * normalize(dispCorrected);
                                    player.position = collisionPoint + dispCorrected;
                                }
                                else
                                {
                                    player.position = collisionPoint;
                                    break;
                                }
                            }
                            while(dtr > 0 && levelRectCollides(&level,
                                                               playerSize, player.position, player.velocity, dtr,
                                                               collisionPoint,normal ));
                            
                            if(levelRectCollides(&level,
                                                 playerSize, player.position, player.velocity, dt,
                                                 collisionPoint,normal ))
                            {
                                player.position = collisionPoint;
                            }
                        }
                        else
                        {
                            player.position = newPos;
                        }
                        
                        // Stay inside the Level
                        player.position = max(player.position,0.5f*playerSize);
                        player.position = min(player.position,Vec2(level.width,level.height)-0.5f*playerSize);
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
                Texture* playerTexture;
                SDL_Surface* playerBitmap;
                if(player.velocity.x > 0 || player.velocity.y > 0)
                {
                    playerTexture = &player.textures[1];
                    playerBitmap = player.bitmaps[1];
                }
                else
                {
                    playerTexture = &player.textures[0];
                    playerBitmap = player.bitmaps[0];
                }
 
#ifndef RENDER_GL
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
                                rect.x = i * level.pixelPerUnit;
                                rect.y = j * level.pixelPerUnit;
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
                        bulletRect.y = levelUnitToPixel(pixPerUnit,  level.height-bullets[i].position.y - 0.5f*bulletSize.y);
                        SDL_BlitSurface(bulletBitmap,0,window_surface,&bulletRect);
                    }
                    
                    // Player
                    SDL_Rect playerRect;
                    playerRect.w = levelUnitToPixel(pixPerUnit, player.size);
                    playerRect.h = levelUnitToPixel(pixPerUnit, player.size);
                    playerRect.x = levelUnitToPixel(pixPerUnit, player.position.x-0.5f*player.size);
                    playerRect.y = levelUnitToPixel(pixPerUnit, level.height-player.position.y-0.5f*player.size);
                    
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
                    static SDL_Surface* redSurface = 0;
                    if(!redSurface)
                    {
                        redSurface = SDL_CreateRGBSurface(0, 64, 64, 32,0,0,0,0);
                        memset(redSurface->pixels, 255<<16, sizeof(uint32)*64*64);
                    }
                    
                    for (uint32 j = 0; j < level.height; j++)
                    {
                        for (uint32 i = 0; i < level.width; i++)
                        {
                            uint8 value = levelValueAtTile(&level,i,j);
                            if(value == WALL)
                            {
                                real32 pixPerUnit = level.pixelPerUnit;
                                SDL_Rect debugPlayerRect;
                                debugPlayerRect.w = levelUnitToPixel(pixPerUnit, 1);
                                debugPlayerRect.h = levelUnitToPixel(pixPerUnit, 1);
                                debugPlayerRect.x = levelUnitToPixel(pixPerUnit, i);
                                debugPlayerRect.y = levelUnitToPixel(pixPerUnit, j);
                                
                                SDL_BlitScaled(debugSurface,0,window_surface,&debugPlayerRect);
                            }
                        }
                    }
                    
                    // Collision rect
                    real32 pixPerUnit = level.pixelPerUnit;
                    SDL_Rect debugPlayerRect;
                    debugPlayerRect.w = levelUnitToPixel(pixPerUnit, playerSize.x);
                    debugPlayerRect.h = levelUnitToPixel(pixPerUnit, playerSize.y);
                    debugPlayerRect.x = levelUnitToPixel(pixPerUnit, player.position.x - 0.5f*playerSize.x);
                    debugPlayerRect.y = levelUnitToPixel(pixPerUnit, level.height - player.position.y - 0.5f*playerSize.y);

                    SDL_BlitScaled(debugSurface,0,window_surface,&debugPlayerRect);
                }
                
#endif
                
                // swap
                SDL_UpdateWindowSurface(window);
#endif
                
#ifdef RENDER_GL
                
                glClearColor(0.f, 0.f, 0.f, 1.f);
                glClear(GL_COLOR_BUFFER_BIT);
                glEnable(GL_BLEND);
                glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
                
                glUseProgram(spriteShader.progId);
                
                real32 mat[16];
                ortho(mat, 0, level.width, 0, level.height, -1, 1);
                glUniformMatrix4fv(spriteShader.projLoc, 1, true, mat);
                
                glBindBuffer(GL_ARRAY_BUFFER, spriteMesh.vboId);
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(6 * sizeof(Vec2)));
                
                // Bind the texture
                glActiveTexture(GL_TEXTURE0);
                glUniform1i(spriteShader.texLoc, 0);
                
                
                // Ground
                glBindTexture(GL_TEXTURE_2D, groungTexture.texId);
                glUniform2f(spriteShader.sizeLoc, level.width, level.height);
                glUniform2f(spriteShader.posLoc, 0.5*level.width, 0.5f*level.height);
                glDrawArrays(GL_TRIANGLES, 0, 6);

                // Walls
                glUniform2f(spriteShader.sizeLoc,1,1);
                for (uint32 j = 0; j < level.height; j++)
                {
                    for (uint32 i = 0; i < level.width; i++)
                    {
                        uint8 value = levelValueAtTile(&level,i,j);
                        glBindTexture(GL_TEXTURE_2D, gameEntityTextures[value].texId);
                        glUniform2f(spriteShader.posLoc, i+0.5f ,j+0.5f);
                        glDrawArrays(GL_TRIANGLES, 0, 6);
                    }
                }
                
                // Bullets
                glUniform2fv(spriteShader.sizeLoc, 1, &bulletSize.x);
                glBindTexture(GL_TEXTURE_2D, bulletTexture.texId);
                for (uint32 i = 0; i < bulletCount; i++)
                {
                    glUniform2fv(spriteShader.posLoc, 1, &bullets[i].position.x);
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                }
                
                // Player
                glUniform2f(spriteShader.sizeLoc, player.size, player.size);
                glUniform2fv(spriteShader.posLoc, 1, &player.position.x);
                glBindTexture(GL_TEXTURE_2D, playerTexture->texId);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                
                glDisableVertexAttribArray(0);
                glUseProgram(0);
                
                SDL_GL_SwapWindow(window);
                //logOpenGLErrors();
#endif
                
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
