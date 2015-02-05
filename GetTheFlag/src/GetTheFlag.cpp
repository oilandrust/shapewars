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
#define DEVENV


// TODO: Free memory
// TODO: Multiple Monitor
// TODO: Understand how flip and blit works with monitor refresh sync
// TODO: Improve the timer (fixed dt)
// TODO: Rotate sprites
// TODO: Create shader, VBuffer


void drawEntitiesVertexColor(Entity* entities, uint32 count, Vec3 size,
                             GLuint sizeLoc, GLuint posLoc ,
                             GLuint vao, uint32 indexCount)
{
    glBindVertexArray(vao);
    glUniform3fv(sizeLoc,1,&size.x);
    for (uint32 i = 0; i < count; i++) {
        glUniform3f(posLoc, entities[i].position.x, entities[i].position.y, 1.f);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }
}

#include <unistd.h>
#define MAXPATHLEN 1024
#include <mach-o/dyld.h>
#include <cerrno>

int main()
{

#ifndef DEVENV
    // Change the current directory
    // Because on mac the current directory might be different than the executable directory
    {
        char exePath[MAXPATHLEN];
        char path[MAXPATHLEN];
        uint32_t size = sizeof(path);
        if(_NSGetExecutablePath(path,&size) == 0)
        {
            realpath(path, exePath);
            LOG("Executable path: %s\n", exePath);
        }
        
        char exeDirectory[MAXPATHLEN];
        memcpy(exeDirectory, exePath, strlen(exePath));
        char* lastSlash = strrchr(exeDirectory, '/');
        *lastSlash = '\0';
        
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        LOG("Current working directory: %s\n", cwd);
  
        LOG("Setting working directory: %s\n",exeDirectory);
        if(chdir(exeDirectory) == -1)
        {
            LOG("chdir failed to %s\n",exeDirectory);
            LOG("errno %d\n",errno);
        }
    }
#endif
    
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
            ASSERT(window_surface, "SDL_error %s", SDL_GetError());
            
            // Initialze OpenGL 3.1
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
           
            //Create context
            SDL_GLContext glContext = SDL_GL_CreateContext(window);
            ASSERT(!glGetError(), "Error Creating Contex");
            ASSERT(glContext, "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
            
            //Initialize GLEW
            glewExperimental = GL_TRUE;
            GLenum result = glewInit();
            ASSERT(result == GLEW_OK, "Error initializing GLEW! %s", glewGetErrorString(result));
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
            Entities entities;
            Level level;
            if(!loadLevel(&level, &entities, "data/lvl2.bmp"))
            {
                printf("loadLevel: %s\n", SDL_GetError());
                ASSERT(false, "%s", SDL_GetError());
            }

            // Load background bitmap
            Texture groungTexture;
            loadTexture(&groungTexture, "data/bg.bmp");
            
            Player player;
            initializePlayer(&player);
            
            // TODO: Turn this into a proper memory pool
            Vec3 meshData[50*4096];
            
            Mesh3D boxMesh;
            void* top = createCube(&boxMesh, (void*)meshData);
            GLuint boxVao = create3DVertexArray(&boxMesh);
            
            Mesh3D heartMesh;
            top = loadObjMesh(&heartMesh, (void*)top, "data/heart_low_c.obj",true);
            GLuint heartVao = create3DVertexArray(&heartMesh);
            
            Mesh3D bombMesh;
            top = loadObjMesh(&bombMesh, (void*)top, "data/bomb.obj",true);
            GLuint bombVao = create3DVertexArray(&bombMesh);
            
            Mesh3D machineGunMesh;
            top = loadObjMesh(&machineGunMesh, (void*)top, "data/ak47.obj",true);
            GLuint machineGunVao = create3DVertexArray(&machineGunMesh);
            
            Mesh3D shotGunMesh;
            top = loadObjMesh(&shotGunMesh, (void*)top, "data/shotgun.obj",true);
            GLuint shotGunVao = create3DVertexArray(&shotGunMesh);
            
            Mesh3D playerMesh;
            top = loadObjMesh(&playerMesh, (void*)top, "data/mario.obj");
            GLuint playerVao = create3DVertexArray(&playerMesh);

            
            Texture marioTexture;
            loadTexture(&marioTexture, "data/mario_main.bmp");
            
            Texture wallTexture;
            loadTexture(&wallTexture, "data/brick.bmp");
            
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
                    
                    // Render Begin
                    {
                        glClearDepth(1.0f);
                        glEnable(GL_DEPTH_TEST);
                        glDepthMask(GL_TRUE);
                        glDepthFunc(GL_LEQUAL);
                        glDepthRange(0.0f, 1.0f);
                        
                        glCullFace(GL_BACK);
                        glEnable(GL_CULL_FACE);
                        
                        
                        // Set Global render states
                        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        glEnable(GL_BLEND);
                        
                        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
                        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
                        logOpenGLErrors();
                        
                        // Go
                        glClear(GL_COLOR_BUFFER_BIT);
                    }
                    
                    // Set Global Uniforms
                    // Set The perspective matrix
                    
                    real32 mat[16];
                    real32 aspect =  (real32)ScreenWidth/(real32)ScreenHeight;
                    perspective(mat, 40.f, aspect, 1.f, 20.f);
                    
                    // Set the view Matrix
                    Vec3 staticCameraPosition(0.3*level.width, -0.6*level.height, 20);
                    Vec3 staticCameraTarget(0.45f*level.width, 0.3f*level.height, 0);
                    
                    Vec3 cameraTarget = staticCameraTarget;
                    Vec3 cameraPosition = staticCameraPosition;
                    
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
                        glActiveTexture(GL_TEXTURE0);
                        glUniform1i(spriteShader->texLoc, 0);
                        
                        // Rot Matrix
                        real32 rot[4] = {
                            1.f, 0.f,
                            0.f, 1.f
                        };
                        glUniformMatrix2fv(spriteShader->rotLoc, 1, false, rot);
                        
                        // Ground
                        glBindTexture(GL_TEXTURE_2D, groungTexture.texId);
                        glUniform2f(spriteShader->sizeLoc, level.width, level.height);
                    
                        glUniform2f(spriteShader->posLoc, 0.5*level.width, -0.5f*level.height);
                        glDrawArrays(GL_TRIANGLES, 0, 6);
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
                    }
                    logOpenGLErrors();
                    
                    
                    /*
                     * DRAW 3D COLOR VERTEX
                     */
                    Shader* vertexDiffuseShader = &renderer.vertexDiffuseShader;
                    glUseProgram(vertexDiffuseShader->progId);
                    glUniformMatrix4fv(vertexDiffuseShader->projLoc, 1, true, mat);
                    glUniformMatrix4fv(vertexDiffuseShader->viewLoc, 1, true, viewMatrix.data);
                    
                    glUniform3f(vertexDiffuseShader->sizeLoc,1,1,1);
                    
                    static real32 hearRot = 0;
                    hearRot += dt;
                    Mat3 rot;
                    identity(rot);
                    rotationZ(rot, hearRot);
                    glUniformMatrix3fv(vertexDiffuseShader->rotLoc, 1, true, rot.data);
                    
                    drawEntitiesVertexColor(entities.heartPickups, entities.nHearts, Vec3(1.f,1.f,1.f),
                                            vertexDiffuseShader->sizeLoc, vertexDiffuseShader->posLoc,
                                            heartVao, 3*heartMesh.fCount);
                    
                    drawEntitiesVertexColor(entities.bombPickups, entities.nBombs, Vec3(1.f,1.f,1.f),
                                            vertexDiffuseShader->sizeLoc, vertexDiffuseShader->posLoc,
                                            bombVao, 3*bombMesh.fCount);
                    
                    drawEntitiesVertexColor(entities.mgPickups, entities.nMG, Vec3(2.f,2.f,2.f),
                                            vertexDiffuseShader->sizeLoc, vertexDiffuseShader->posLoc,
                                            machineGunVao, 3*machineGunMesh.fCount);
                    
                    drawEntitiesVertexColor(entities.sgPickups, entities.nSG, Vec3(2.f,2.f,2.f),
                                            vertexDiffuseShader->sizeLoc, vertexDiffuseShader->posLoc,
                                            shotGunVao, 3*shotGunMesh.fCount);

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
                    playerOrientation.data[3] = player.aimDir.y;
                    playerOrientation.data[1] = -player.aimDir.y;
                    playerOrientation.data[4] = player.aimDir.x;
                    
                    glUniformMatrix3fv(texturedDiff->rotLoc, 1, true, playerOrientation.data);
                    
                    glBindVertexArray(playerVao);
    
                    glBindTexture(GL_TEXTURE_2D, marioTexture.texId);
                    
                    glUniform3f(texturedDiff->sizeLoc, 3, 3, 3);
                    glUniform3f(texturedDiff->posLoc, player.position.x, player.position.y, 0);
                    
                    glDrawElements(GL_TRIANGLES, 3*playerMesh.fCount, GL_UNSIGNED_INT, 0);
                    logOpenGLErrors();
                    
                    
                    // WALL
                    glBindTexture(GL_TEXTURE_2D, wallTexture.texId);
                    
                    Mat3 orientation;
                    identity(orientation);
                    
                    glUniformMatrix3fv(texturedDiff->rotLoc, 1, false, orientation.data);
                    
                    glBindVertexArray(boxVao);
                    glUniform3f(texturedDiff->sizeLoc,1,1,2.f);
                    
                    uint32 count = entities.nWalls;
                    Entity* ent = entities.walls;
                    for (uint32 i = 0; i < count; i++) {
                        glUniform3f(texturedDiff->posLoc, ent[i].position.x, ent[i].position.y, 1.f);
                        glDrawElements(GL_TRIANGLES, 3*boxMesh.fCount, GL_UNSIGNED_INT, 0);
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
