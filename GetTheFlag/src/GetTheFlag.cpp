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
#include "Animation.h"
#include "NavMeshGen.h"

#define DEVENV

#include <unistd.h>
#define MAXPATHLEN 1024
#include <mach-o/dyld.h>
#include <cerrno>

void drawBox(Shader* shader, Mesh3D* mesh, Vec3 pos, Vec3 size) {
    Mat3 rot;
    identity(rot);
    
    glUniformMatrix3fv(shader->rotLoc, 1, true, rot.data);
    glUniform3f(shader->sizeLoc, size.x, size.y, size.z);
    glUniform3f(shader->posLoc, pos.x, pos.y, pos.z);
    glUniform3f(shader->diffuseLoc, 1.0f, 1.0f, 1.0f);
    
    glDrawElements(GL_TRIANGLES, 3*mesh->fCount, GL_UNSIGNED_INT, 0);

}

#define MAP_WIDTH 32
#define MAP_HEIGHT 32

uint8 levelMap[MAP_WIDTH*MAP_HEIGHT] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
    2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
    2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

struct ViewCamera {
    Mat4 projection;
    Mat4 view;
};

struct Ray {
    Vec3 origin;
    Vec3 direction;
};

Vec3 intersectGround0(const Ray& ray) {
    // (p0 + t n0) . Z = 0
    // t = -p0.z / n0.z
    real32 t = - ray.origin.z / ray.direction.z;
    return ray.origin + t * ray.direction;
}

Ray unproject(ViewCamera* camera, Vec2 /*screenPos*/) {
    Ray result;
    
    result.origin.x = camera->view.data[3];
    result.origin.y = camera->view.data[7];
    result.origin.z = camera->view.data[11];
        
    Vec3 forward = Vec3(-camera->view.data[2],
                        -camera->view.data[6],
                        -camera->view.data[10]);
    result.direction = forward;
    
    return result;
}

int main()
{

#ifndef DEVENV
    // Change the current directory
    // Because on mac the current directory might be different than the executable directory
    {
        char exePath[MAXPATHLEN];
        char path[MAXPATHLEN];
        uint32_t size = sizeof(path);
        if (_NSGetExecutablePath(path, &size) == 0) {
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

        LOG("Setting working directory: %s\n", exeDirectory);
        if (chdir(exeDirectory) == -1) {
            LOG("chdir failed to %s\n", exeDirectory);
            LOG("errno %d\n", errno);
        }
    }
#endif

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        return 0;
    }
    
    uint32 ScreenWidth = 640;
    uint32 ScreenHeight = 480;

    uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;
    SDL_Window* window = SDL_CreateWindow("Box War", 0, 0, ScreenWidth, ScreenHeight, windowFlags);

    if (!window) {
        return 0;
    }
    
    SDL_Surface* window_surface = 0;

    int32 monitorRefreshRate = 30;
    int32 gameRefreshRate = monitorRefreshRate;
    real32 targetMsPerFrame = 1000.0f / gameRefreshRate;

    window_surface = SDL_GetWindowSurface(window);
    ASSERT_MSG(window_surface, "SDL_error %s", SDL_GetError());

    // Initialze OpenGL 3.1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    //Create context
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    ASSERT_MSG(!glGetError(), "Error Creating Contex");
    ASSERT_MSG(glContext, "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());

    //Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum result = glewInit();
    ASSERT_MSG(result == GLEW_OK, "Error initializing GLEW! %s", glewGetErrorString(result));
    glGetError();

    //Use Vsync
    if (SDL_GL_SetSwapInterval(1) < 0) {
        printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
    }
    
    size_t memSize = Megabytes(512);
    MemoryArena memoryArena;
    initializeArena(&memoryArena, new uint8[memSize], memSize);

    // Initialize the rendering resources
    Renderer renderer;
    intializeRendererRessources(&renderer);

    // Load The Level
    Level level;
    level.width = MAP_WIDTH;
    level.height = MAP_HEIGHT;
    level.tiles = levelMap;
    
    // Init the nav mesh.
    DistanceField distanceField;
    genDistanceField(&memoryArena, &level, &distanceField);
    
    RegionIdMap regionIds;
    genRegions(&memoryArena, &distanceField, &regionIds);
    
    Contours contour;
    genContours(&memoryArena, &regionIds, &contour);
    
    Mesh3D* contourMeshes = pushArray<Mesh3D>(&memoryArena, contour.count);
    GLuint* lineVaos = pushArray<GLuint>(&memoryArena, contour.count);
    for(uint32 i = 0; i < contour.count; i++) {
        contourMeshes[i].vCount = contour.contours[i].count;
        contourMeshes[i].positions = contour.contours[i].vertices;
        contourMeshes[i].normals = 0;
        contourMeshes[i].indices = 0;
        contourMeshes[i].uvs = 0;
        contourMeshes[i].colors = 0;
        lineVaos[i] = create3DVertexArray(&contourMeshes[i]);
    }
    
    bool DebugShowDistanceField = false;
    bool DebugShowRegions = false;
    
    CameraPan camera;
    initializeCameraPan(&camera);
    camera.target = Vec3(0.5*MAP_WIDTH, 0.3*MAP_HEIGHT, 0);

    AIEntity bot;
    
    ViewCamera viewCamera;
    real32 aspect = (real32)ScreenWidth / (real32)ScreenHeight;
    perspective(viewCamera.projection, 40.f, aspect, 1.f, 200.f);
    
    Mesh3D boxMesh;
    createCube(&memoryArena, &boxMesh);
    GLuint boxVao = create3DIndexedVertexArray(&boxMesh);
    
    Mesh3D planeMesh;
    createPlane(&memoryArena, &planeMesh);
    GLuint planeVao = create3DIndexedVertexArray(&planeMesh);
    
    Input input;
    memset(&input, 0, sizeof(input));

    // Timers
    uint64 lastCounter = SDL_GetPerformanceCounter();
    uint64 endCounter = 0;
    uint64 counterFrequency = SDL_GetPerformanceFrequency();

    // Check error at initialization

    ASSERT_MSG(!glGetError(), "OpenGl Error after initialization");
    logOpenGLErrors();

    bool running = true;
    // The main Loop
    while (running) {

        // Time delta in seconds
        real32 dt = targetMsPerFrame / 1000.0f;

        // Update the input state
        processInput(&input);

        // Special Input Handling
        // Quit Game
        if (input.keyStates[QUIT].clicked
            || input.keyStates[ESCAPE].clicked) {
            running = false;
        }
        // Reload the shader
        if (input.keyStates[DEBUG_RELOAD_SHADERS].clicked) {
            reloadShaders(&renderer);
        }
      
        if (input.keyStates[DEBUG_SHOW_DISTANCE_FIELD].clicked) {
            DebugShowDistanceField = true;
            DebugShowRegions = false;
        }
        if (input.keyStates[DEBUG_SHOW_REGIONS].clicked) {
            DebugShowRegions = true;
            DebugShowDistanceField = false;
        }

        updateCameraPan(&camera, &input, &level, dt);
        
        Vec2 mousePos(input.mouseX/ScreenWidth - 0.5,
                      input.mouseY/ScreenHeight - 0.5);
        
        Ray mouseRay = unproject(&viewCamera, mousePos);
        Vec3 groundPos = intersectGround0(mouseRay);
        
        if (input.keyStates[FIRE1].clicked) {
            setAIEntityTarget(&level, &bot, groundPos);
        }
        
        updateAIEntity(&bot);
        updateEntity(&bot.entity);
        
        // Render Game
        {
            // Render Begin
            rendererBeginFrame(&renderer);
            
            // Set Global Uniforms

            // Set the view Matrix
            identity(viewCamera.view);
            lookAt(viewCamera.view, camera.position, camera.target, Vec3(0, 0, 1));
            
            Mat4 view = viewCamera.view;
            inverseTransform(view);

            Shader* texDiffShader = &renderer.texDiffShader;
            glUseProgram(texDiffShader->progId);
            glUniformMatrix4fv(texDiffShader->projLoc,
                               1, true, viewCamera.projection.data);
            glUniformMatrix4fv(texDiffShader->viewLoc,
                               1, true, view.data);

            glBindVertexArray(planeVao);

            Mat3 rot;
            identity(rot);
            glUniformMatrix3fv(texDiffShader->rotLoc, 1, true, rot.data);
            glUniform3f(texDiffShader->sizeLoc, MAP_WIDTH, MAP_HEIGHT, 0.0f);
            glUniform3f(texDiffShader->posLoc, 0.5f * MAP_WIDTH, 0.5f * MAP_HEIGHT, 0.f);
            
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(texDiffShader->diffTexLoc, 0);
            
            if(DebugShowDistanceField)
                glBindTexture(GL_TEXTURE_2D, distanceField.texture.texId);
            else
                glBindTexture(GL_TEXTURE_2D, regionIds.texture.texId);
            
            glDrawElements(GL_TRIANGLES, 3*planeMesh.fCount, GL_UNSIGNED_INT, 0);

            /*
             * DRAW 3D COLOR VERTEX
             */
            Shader* flatDiffShader = &renderer.flatDiffShader;
            glUseProgram(flatDiffShader->progId);
            glUniformMatrix4fv(flatDiffShader->projLoc,
                               1, true, viewCamera.projection.data);
            glUniformMatrix4fv(flatDiffShader->viewLoc,
                               1, true, view.data);
            
            // Draw lines
            for(uint32 i = 0; i < contour.count; i++) {
                glBindVertexArray(lineVaos[i]);
                identity(rot);
                glUniformMatrix3fv(flatDiffShader->rotLoc, 1, true, rot.data);
                glUniform3f(flatDiffShader->sizeLoc, 1, 1, 1);
                glUniform3f(flatDiffShader->posLoc, 0, 0, 0);
                glUniform3f(flatDiffShader->diffuseLoc, 1.0f, 1.0f, 1.0f);
                
                glDrawArrays(GL_LINE_STRIP, 0, contour.contours[i].count);
            }
            // end lines
            
            glBindVertexArray(boxVao);

            for(uint32 j = 0; j < MAP_HEIGHT; j++) {
                for(uint32 i = 0; i < MAP_WIDTH; i++) {
                    uint32 val = levelMap[i+j*MAP_HEIGHT];
                    if(val) {
//                        Vec3 pos = Vec3(i+0.5f, MAP_HEIGHT-j-0.5f, val-0.5);
//                        drawBox(flatDiffShader, &boxMesh, pos, Vec3(1.f, 1.f, 1.f));
                    }
                }
            }
            drawBox(flatDiffShader, &boxMesh,
                    bot.entity.position + Vec3(0.f,0.f,.5f),
                    Vec3(0.5f, 0.5f, 0.5f));

            drawBox(flatDiffShader, &boxMesh,
                    groundPos, Vec3(.25f, .25f, 1.f));
            
            for(uint32 i = 0; i < bot.pathLength; i++) {
                Vec3 pos(bot.path[i]%MAP_WIDTH+0.5f, bot.path[i]/MAP_WIDTH+0.5f, 0);
                drawBox(flatDiffShader, &boxMesh, pos, Vec3(.8f, .8f, 0.1f));
            }

            logOpenGLErrors();

#if 0
            {
                // RENDER DEBUG CODE HERE
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

                    // Camera follow zone
                    glBindTexture(GL_TEXTURE_2D, groungTexture.texId);
                    glUniform2f(spriteShader->sizeLoc, camera.zoneSize.x, camera.zoneSize.y);
                    glUniform2f(spriteShader->posLoc, camera.target.x, camera.target.y);
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                }
            }
#endif

            glBindVertexArray(0);
            glUseProgram(0);

            SDL_GL_SwapWindow(window);
            logOpenGLErrors();
        }

        // Update the clock
        endCounter = SDL_GetPerformanceCounter();

        uint64 couterElapsed = endCounter - lastCounter;
        real32 msElapsed = ((1000.0f * (real32)couterElapsed) / (real32)counterFrequency);

        // TODO COMPRESS THIS
        if (msElapsed < targetMsPerFrame) {
            SDL_Delay((uint32)targetMsPerFrame - msElapsed);
            lastCounter = endCounter;
            endCounter = SDL_GetPerformanceCounter();
            couterElapsed = endCounter - lastCounter;
            msElapsed += ((1000.0f * (real32)couterElapsed) / (real32)counterFrequency);
        }

        //int32 fps = 1000/msElapsed;
        //printf("%f ms, %d fps %d\n",msElapsed,fps,bulletCount);
        lastCounter = endCounter;
    }
    
    // Cleanup
    SDL_DestroyWindow(window);


    SDL_Quit();
    return 0;
}
