#include <cstdio>

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include "GetTheFlag.h"
#include "Input.h"
#include "Level.h"
#include "OpenGLClient.h"
#include "Renderer.h"
#include "Entities.h"
#include "Vec3.h"
#include "Mat4.h"
#include "Mat3.h"
#include "Mesh.h"

//#define RENDER_DEBUG
#define DEBUG_GAME


// TODO: Free memory

struct GameMemory {
    Input input;
    Player player;
    Level level;
    BulletManager bullets;
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
        uint32 ScreenWidth = 2*640;
        uint32 ScreenHeight = 2*480;
        
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
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
           
            //Create context
            SDL_GLContext glContext = SDL_GL_CreateContext(window);
            ASSERT(!glGetError(), "Error Creating Contex");
            ASSERT(glContext, "OpenGL context could not be created! SDL Error: %s\n" + std::string(SDL_GetError()));
            
            //Initialize GLEW
            glewExperimental = GL_TRUE;
            GLenum result = glewInit();
            ASSERT(result == GLEW_OK, "Error initializing GLEW!" + std::string((const char*)glewGetErrorString(result)));
            glGetError();
            
            //Use Vsync
            if( SDL_GL_SetSwapInterval( 1 ) < 0 )
            {
                printf( "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError() );
            }
            
            // Initialize the rendering resources
            Renderer renderer;
            intializeRendererRessources(&renderer);
            
            // Load The Level
            Level level;
            if(!loadLevel(&level,"data/lvl2.bmp"))
            {
                printf("loadLevel: %s\n", IMG_GetError());
                ASSERT(false, IMG_GetError());
            }

            // Load background bitmap
            Texture groungTexture;
            loadTexture(&groungTexture, "data/bg.png");
            
            Player player;
            initializePlayer(&player);
            
            
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

            Vec3 meshData[12*4096];
            
            Mesh3D boxMesh;
            void* top = createCube(&boxMesh, (void*)meshData);
            GLuint boxVao = create3DVertexArray(&boxMesh);
            
            Mesh3D heartMesh;
            top = loadObjMesh(&heartMesh, (void*)top, "data/heart_low_c.obj",true);
            GLuint heartVao = create3DVertexArray(&heartMesh);
            
            Mesh3D bombMesh;
            top = loadObjMesh(&bombMesh, (void*)top, "data/bomb.obj",true);
            GLuint bombVao = create3DVertexArray(&bombMesh);
            
            Mesh3D playerMesh;
            top = loadObjMesh(&playerMesh, (void*)top, "data/mario.obj",true);
            GLuint playerVao = create3DVertexArray(&playerMesh);
            
            Texture marioTexture;
            loadTexture(&marioTexture, "data/mario_main.png");
            
            BulletManager bulletManager;
            initializeBullets(&bulletManager);
            
            Input input;
            memset(&input, 0, sizeof(input));
            
            // Timers
            uint64 lastCounter = SDL_GetPerformanceCounter();
            uint64 endCounter = 0;
            uint64 counterFrequency = SDL_GetPerformanceFrequency();
            
            // Check error at initialization
            
            ASSERT(!glGetError(), "OpenGl Error after initialization");
            logOpenGLErrors();
            
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
                        reloadShaders(&renderer);
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
                    // TODO: Uniform buffer for matrixes
                    // TODO: Refactor
                    
                    glClearDepth(1.0f);
                    glDisable(GL_DEPTH_TEST);
                    glDepthMask(GL_TRUE);
                    glDepthFunc(GL_LEQUAL);
                    glDepthRange(0.0f, 1.0f);
                    
                    
                    // Set Global render states
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    glEnable(GL_BLEND);
                    
                    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
                    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
                    logOpenGLErrors();
                    
                    // Go
                    glClear(GL_COLOR_BUFFER_BIT);
                    
                    //
                    // Set The perspective matrix
                    real32 mat[16];
                    real32 aspect =  (real32)ScreenWidth/(real32)ScreenHeight;
                    perspective(mat, 60.f, aspect, 1.f, 20.f);
                    
                    // Set the view Matrix
                    Vec3 cameraPosition(0.4*level.width, -0.6*level.height, 15);
                    Vec3 cameraTarget(0.5f*level.width, 0.5f*level.height, 0);
                    
                    Mat4 viewMatrix;
                    identity(viewMatrix);
                    lookAt(viewMatrix, cameraPosition, cameraTarget, Vec3(0,0,1));
                    inverseTransform(viewMatrix);

                    // RENDER 2D
                    {
                        Shader* spriteShader = &renderer.spriteShader;
                        glUseProgram(spriteShader->progId);
                        glUniformMatrix4fv(spriteShader->projLoc, 1, true, mat);
                        glUniformMatrix4fv(spriteShader->viewLoc, 1, true, viewMatrix.data);

                        // Bind the vao for drawing
                        Mesh2* spriteMesh = &renderer.spriteMesh;
                        glBindVertexArray(spriteMesh->vaoId);
                        
                        // Bind the texture to uniform
                        {
                            glActiveTexture(GL_TEXTURE0);
                            glUniform1i(spriteShader->texLoc, 0);
                        }
                        
                        // Rot Matrix
                        real32 rot[4] = {
                            1.f, 0.f,
                            0.f, 1.f
                        };
                        glUniformMatrix2fv(spriteShader->rotLoc, 1, false, rot);
                        
                        // Ground
                        glBindTexture(GL_TEXTURE_2D, groungTexture.texId);
                        glUniform2f(spriteShader->sizeLoc, level.width, level.height);
                    
                        glUniform2f(spriteShader->posLoc, 0.5*level.width, 0.5f*level.height);
                        glDrawArrays(GL_TRIANGLES, 0, 6);
                        glUniform2f(spriteShader->posLoc, 0.5*level.width, 1.5f*level.height);
                        glDrawArrays(GL_TRIANGLES, 0, 6);
                        glUniform2f(spriteShader->posLoc, 1.5*level.width, 0.5f*level.height);
                        glDrawArrays(GL_TRIANGLES, 0, 6);
                        glUniform2f(spriteShader->posLoc, 1.5*level.width, 1.5f*level.height);
                        glDrawArrays(GL_TRIANGLES, 0, 6);
                        glUniform2f(spriteShader->posLoc, -0.5*level.width, 0.5f*level.height);
                        glDrawArrays(GL_TRIANGLES, 0, 6);
                        glUniform2f(spriteShader->posLoc, -0.5*level.width, 1.5f*level.height);
                        glDrawArrays(GL_TRIANGLES, 0, 6);
                        
                        glUniform2f(spriteShader->sizeLoc,1,1);
                        for (uint32 j = 0; j < level.height; j++)
                        {
                            for (uint32 i = 0; i < level.width; i++)
                            {
                                uint8 value = levelValueAtTile(&level,i,j);
                                if (value < MAX_ENTITY_TYPE)
                                {
                                    if(value != WALL && value != HEART && value != BOMB)
                                    {
                                        glBindTexture(GL_TEXTURE_2D, gameEntityTextures[value].texId);
                                        glUniform2f(spriteShader->posLoc, i+0.5f ,j+0.5f);
                                        glDrawArrays(GL_TRIANGLES, 0, 6);
                                    }
                                }
                            }
                        }
                    }
                    logOpenGLErrors();
                    
                    
                    glEnable(GL_DEPTH_TEST);
                    /*
                     * DRAW 3D COLOR VERTEX
                     */
                    Shader* vertexDiffuseShader = &renderer.vertexDiffuseShader;
                    glUseProgram(vertexDiffuseShader->progId);
                    glUniformMatrix4fv(vertexDiffuseShader->projLoc, 1, true, mat);
                    glUniformMatrix4fv(vertexDiffuseShader->viewLoc, 1, true, viewMatrix.data);
                    
                    glUniform3f(vertexDiffuseShader->sizeLoc,1,1,1);
                    
                    for (uint32 j = 0; j < level.height; j++)
                    {
                        for (uint32 i = 0; i < level.width; i++)
                        {
                            uint8 value = levelValueAtTile(&level,i,j);
                            if (value < MAX_ENTITY_TYPE)
                            {
                                if(value == HEART)
                                {
                                    glBindVertexArray(heartVao);
                                    glUniform3f(vertexDiffuseShader->posLoc, i+0.5f ,j+0.5f, 1.f);
                                    glDrawElements(GL_TRIANGLES, 3*heartMesh.fCount, GL_UNSIGNED_INT, 0);
                                }
                                if(value == BOMB)
                                {
                                    glBindVertexArray(bombVao);
                                    glUniform3f(vertexDiffuseShader->posLoc, i+0.5f ,j+0.5f, 1.f);
                                    glDrawElements(GL_TRIANGLES, 3*bombMesh.fCount, GL_UNSIGNED_INT, 0);
                                }
                            }
                        }
                    }
                    logOpenGLErrors();

                    
                    /*
                     * DRAW 3D COLOR TEXTURED
                     */
                    // PLAYER
                    Shader* texturedDiff = &renderer.textureDiffuseShader;
                    glUseProgram(texturedDiff->progId);
                    glUniformMatrix4fv(texturedDiff->projLoc, 1, true, mat);
                    glUniformMatrix4fv(texturedDiff->viewLoc, 1, true, viewMatrix.data);
        
                    glActiveTexture(GL_TEXTURE0);
                    glUniform1i(texturedDiff->diffuseLoc, 0);
                    
                    Mat3 playerOrientation;
                    identity(playerOrientation);
                    playerOrientation.data[0] = player.aimDir.x;
                    playerOrientation.data[1] = player.aimDir.x;
                    playerOrientation.data[2] = player.aimDir.x;
                    playerOrientation.data[4] = player.aimDir.x;
                    
                    glBindVertexArray(playerVao);
        
                    {
                        glBindTexture(GL_TEXTURE_2D, marioTexture.texId);
                        
                        glUniform3f(texturedDiff->sizeLoc, 3, 3, 3);
                        glUniform3f(texturedDiff->posLoc, player.position.x, player.position.y, 0);
                        
                        glDrawElements(GL_TRIANGLES, 3*playerMesh.fCount, GL_UNSIGNED_INT, 0);
                    }
                    logOpenGLErrors();
                    
                    
                    // WALL
                    glUniform3f(texturedDiff->sizeLoc,1,1,2.f);
                    glBindTexture(GL_TEXTURE_2D, gameEntityTextures[WALL].texId);
                    
                    glBindVertexArray(boxVao);
                    
                    for (uint32 j = 0; j < level.height; j++)
                    {
                        for (uint32 i = 0; i < level.width; i++)
                        {
                            uint8 value = levelValueAtTile(&level,i,j);
                            if (value < MAX_ENTITY_TYPE)
                            {
                                if(value == WALL)
                                {
                                    glUniform3f(texturedDiff->posLoc, i+0.5f ,j+0.5f, 1.f);
                                    glDrawElements(GL_TRIANGLES, 3*boxMesh.fCount, GL_UNSIGNED_INT, 0);
                                }
                            }
                        }
                    }
                    logOpenGLErrors();
                    
                    // BULLETS
                    {
                        Texture* bulletTexture = &bulletManager.bulletTexture;
                        uint32 bulletCount = bulletManager.bulletCount;
                        Bullet* bullets = bulletManager.bullets;
                        Vec2 bulletSize = bulletManager.bulletSize;
                        
                        glUniform3f(texturedDiff->sizeLoc, bulletSize.x, bulletSize.y, bulletSize.y);
                        glBindTexture(GL_TEXTURE_2D, bulletTexture->texId);
                        for (uint32 i = 0; i < bulletCount; i++)
                        {
                            glUniform3f(texturedDiff->posLoc, bullets[i].position.x, bullets[i].position.y, 0.5f);
                            glDrawElements(GL_TRIANGLES, 3*boxMesh.fCount, GL_UNSIGNED_INT, 0);
                        }
                    }
                
                    glBindVertexArray(0);
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
