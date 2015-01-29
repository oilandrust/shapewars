#include <cstdio>

#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>

#include "GetTheFlag.h"
#include "Input.h"
#include "Level.h"
#include "OpenGLClient.h"
#include "Entities.h"

//#define RENDER_DEBUG
#define DEBUG_GAME


// TODO: Free memory

struct GameMemory {
    Input input;
    Player player;
    Level level;
    BulletManager bullets;
};

struct Renderer {
    Mesh spriteMesh;
    Shader spriteShader;
};

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
            player.drag = 20.0f;
            player.acc = 200.0f;
            player.position = Vec2(3,3);
            player.velocity = Vec2(0,0);
            player.aimDir = Vec2(1,0);
            player.size = 3.0f;
            player.collisionSize = Vec2(0.4*player.size, player.size);
            
            // Load player bitmap
            loadTexture(&player.textures[0], "data/player1_right_standing.png");
            loadTexture(&player.textures[1], "data/player1_right_walking_1.png");
            loadTexture(&player.textures[2], "data/player1_right_walking_2.png");
            
            // Load background bitmap
            Texture groungTexture;
            loadTexture(&groungTexture, "data/bg.png");
            ASSERT(success, "Initializing texture failed");
            
            // Load Level element bitmaps
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
                loadTexture(&gameEntityTextures[i], gameEntityImageFilename[i]);
            }
            
            // Load Other bitmaps
            Texture bulletTexture;
            loadTexture(&bulletTexture, "data/bullet.png");
            
            // Pool for bullets
            BulletManager bulletManager;
            Bullet bullets[MAX_BULLET_COUNT];
            bulletManager.bullets = bullets;
            bulletManager.bulletCount = 0;
            bulletManager.bulletSpeed = 30.0f;
            bulletManager.bulletSize = Vec2(bulletTexture.width/level.pixelPerUnit,
                                            bulletTexture.height/level.pixelPerUnit);
            
            Input input;
            memset(&input, 0, sizeof(input));
            
            // Timers
            uint64 lastCounter = SDL_GetPerformanceCounter();
            uint64 endCounter = 0;
            uint64 counterFrequency = SDL_GetPerformanceFrequency();
            
            // Check error at initialization
            logOpenGLErrors();
            ASSERT(!glGetError(), "Error");
            
            bool running = true;
            // The main Loop
            while (running)
            {
                
                // Time delta in seconds
                real32 dt = targetMsPerFrame/1000.0f;
                
                // Update the input state
                processInput(&input);
                
                // Special Input Handling
                {
                    // Quit Game
                    if(input.keyStates[QUIT].clicked
                       || input.keyStates[ESCAPE].clicked)
                    {
                        running = false;
                    }
#ifdef DEBUG_GAME
                    // Reload the shader
                    if(input.keyStates[RELOAD].clicked)
                    {
                        glDeleteShader(spriteShader.progId);
                        createShaderProgram(&spriteShader,"shaders/sprite.vs","shaders/sprite.fs");
                        spriteShader.projLoc = glGetUniformLocation(spriteShader.progId, "projection");
                        spriteShader.posLoc = glGetUniformLocation(spriteShader.progId, "entity_position");
                        spriteShader.sizeLoc = glGetUniformLocation(spriteShader.progId, "entity_size");
                        spriteShader.texLoc = glGetUniformLocation(spriteShader.progId, "sprite_texture");
                    }
#endif
                }
                    
                // Update the game entities
                {
                    
                    updatePlayer(&player,&input,&level,dt);
                    if(input.keyStates[FIRE1].clicked)
                    {
                        uint32 bulletIndex = createBullet(&bulletManager);
                        bulletManager.bullets[bulletIndex].position = player.position;
                        bulletManager.bullets[bulletIndex].velocity = player.aimDir;
                    }
                    
                    updateBullets(&bulletManager, &level, dt);
                }
                
                // Render Game
                {
                    // Set Global render states
                    glClearColor(0.f, 0.f, 0.f, 1.f);
                    glEnable(GL_BLEND);
                    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
                    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
                    
                    // Go
                    glClear(GL_COLOR_BUFFER_BIT);
                    
                    //
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
                    {
                        uint32 bulletCount = bulletManager.bulletCount;
                        Bullet* bullets = bulletManager.bullets;
                        Vec2 bulletSize = bulletManager.bulletSize;
                        
                        glUniform2fv(spriteShader.sizeLoc, 1, &bulletSize.x);
                        glBindTexture(GL_TEXTURE_2D, bulletTexture.texId);
                        for (uint32 i = 0; i < bulletCount; i++)
                        {
                            glUniform2fv(spriteShader.posLoc, 1, &bullets[i].position.x);
                            glDrawArrays(GL_TRIANGLES, 0, 6);
                        }
                    }
                    
                    // Player
                    glUniform2f(spriteShader.sizeLoc, player.size, player.size);
                    glUniform2fv(spriteShader.posLoc, 1, &player.position.x);
                    glBindTexture(GL_TEXTURE_2D, player.currentTexture->texId);
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                    
                    glDisableVertexAttribArray(0);
                    glUseProgram(0);
                    
                    SDL_GL_SwapWindow(window);
                    logOpenGLErrors();
                }
                
#ifdef RENDER_DEBUG
                {
                    // RENDER DEBUG CODE HERE
                }
#endif
                
                
                
                // Update the clock
                endCounter = SDL_GetPerformanceCounter();
                
                uint64 couterElapsed = endCounter - lastCounter;
                real32 msElapsed = ((1000.0f * (real32)couterElapsed)/(real32)counterFrequency);
                
                // TODO COMPRESS THIS
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
