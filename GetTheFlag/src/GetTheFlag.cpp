#include <cstdio>

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include "Animation.h"
#include "Entities.h"
#include "GetTheFlag.h"
#include "Input.h"
#include "Level.h"
#include "Mat3.h"
#include "Mat4.h"
#include "Mesh.h"
#include "NavMeshGen.h"
#include "NavMeshQuery.h"
#include "OpenGLClient.h"
#include "Renderer.h"
#include "Vec3.h"

// TODO: 7 days
// NavMesh navigation
// Rendering refactoring
// Debug refactoring
// Mouse unproject
// Cleanup
// Bigger level
// Fog of war
// Minimap
// Igui
// Start Screen
// Pause Menu
// Multiple entities

#define DEVENV

#include <cerrno>
#include <mach-o/dyld.h>
#include <unistd.h>

void drawBox(Shader* shader, Mesh3D* mesh, Vec3 pos, Vec3 size)
{
    Mat3 rot;
    identity(rot);

    glUniformMatrix3fv(shader->rotLoc, 1, true, rot.data);
    glUniform3f(shader->sizeLoc, size.x, size.y, size.z);
    glUniform3f(shader->posLoc, pos.x, pos.y, pos.z);
    glUniform3f(shader->diffuseLoc, 1.0f, 1.0f, 1.0f);

    glDrawElements(GL_TRIANGLES, 3 * mesh->fCount, GL_UNSIGNED_INT, 0);
}

struct ViewCamera {
    Mat4 projection;
    Mat4 view;
};

struct Ray {
    Vec3 origin;
    Vec3 direction;
};

Vec3 intersectGround0(const Ray& ray)
{
    // (p0 + t n0) . Z = 0
    // t = -p0.z / n0.z
    real32 t = -ray.origin.z / ray.direction.z;
    return ray.origin + t * ray.direction;
}

Ray unproject(ViewCamera* camera, Vec2 /*screenPos*/)
{
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
    level.tiles = level2;

    // Init the nav mesh.
    LevelRaster raster{ level.tiles, level.width, level.height };
    DistanceField distanceField;
    genDistanceField(&memoryArena, &raster, &distanceField);

    RegionIdMap regionIds;
    genRegions(&memoryArena, &distanceField, &regionIds);

    ContourSet contours;
    genContours(&memoryArena, &regionIds, &contours);

    Mesh3D* contourMeshes = pushArrayZeroed<Mesh3D>(&memoryArena, contours.count);
    GLuint* lineVaos = pushArray<GLuint>(&memoryArena, contours.count);
    for (uint32 i = 0; i < contours.count; i++) {
        contourMeshes[i].vCount = contours.contours[i].count;
        contourMeshes[i].positions = contours.contours[i].vertices;
        lineVaos[i] = createIndexedVertexArray(contourMeshes[i].positions,
            contourMeshes[i].vCount, contourMeshes[i].indices, 3 * contourMeshes[i].fCount);
    }

    Mesh3D* triangulatedCountours = pushArray<Mesh3D>(&memoryArena, contours.count);
    triangulateContours(&memoryArena, &contours, triangulatedCountours);
    GLuint* contourMeshVaos = pushArray<GLuint>(&memoryArena, contours.count);
    for (uint32 i = 0; i < contours.count; i++) {
        contourMeshVaos[i] = createIndexedVertexArray(&triangulatedCountours[i]);
    }

    NavMesh navMesh;
    DualMesh dual;
    buildNavMesh(&memoryArena, &contours, triangulatedCountours, &navMesh, &dual);
    GLuint* navVaos = pushArray<GLuint>(&memoryArena, navMesh.polyCount);
    for (uint32 i = 0; i < navMesh.polyCount; i++) {
        navVaos[i] = createIndexedVertexArray(navMesh.vertices, navMesh.vertCount,
            navMesh.polygons + 2 * i * navMesh.maxVertPerPoly, navMesh.maxVertPerPoly);
    }
    GLuint dualVao = createIndexedVertexArray(dual.vertices, dual.vertCount, dual.indices, dual.indCount);

    struct Debug {
        bool showDistanceField = false;
        bool showRegions = false;
        bool showContours = false;
        bool showTriRegions = false;
        bool showNavMesh = false;
        bool showDual = false;
    };
    Debug debug;

    CameraPan camera;
    initializeCameraPan(&camera);
    camera.target = Vec3(0.5 * MAP_WIDTH, 0.3 * MAP_HEIGHT, 0);

    AIEntity bot;
    bot.entity.position = Vec3(1, 1, 0);
    Path path;
    path.polyPathLength = 0;
    path.polys = pushArray<uint32>(&memoryArena, navMesh.polyCount);
    path.length = 0;
    path.points = pushArray<Vec3>(&memoryArena, navMesh.polyCount + 2);
    setAIEntityPath(&bot, &path);

    GLuint pathVbo = createBufferObject(path.points, navMesh.polyCount + 2, GL_DYNAMIC_DRAW);
    GLuint pathVao;
    glGenVertexArrays(1, &pathVao);
    glBindVertexArray(pathVao);
    bindAttribBuffer(pathVbo, POS_ATTRIB_LOC, 3);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    ViewCamera viewCamera;
    real32 aspect = (real32)ScreenWidth / (real32)ScreenHeight;
    perspective(viewCamera.projection, 40.f, aspect, 1.f, 200.f);

    Mesh3D boxMesh;
    createCube(&memoryArena, &boxMesh);
    GLuint boxVao = createIndexedVertexArray(&boxMesh);

    Mesh3D planeMesh;
    createPlane(&memoryArena, &planeMesh);
    GLuint planeVao = createIndexedVertexArray(&planeMesh);

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
            debug.showDistanceField = !debug.showDistanceField;
            if (debug.showDistanceField)
                debug.showRegions = false;
        }
        if (input.keyStates[DEBUG_SHOW_REGIONS].clicked) {
            debug.showRegions = !debug.showRegions;
            if (debug.showRegions)
                debug.showDistanceField = false;
        }
        if (input.keyStates[DEBUG_SHOW_CONTOURS].clicked) {
            debug.showContours = !debug.showContours;
            if (debug.showContours)
                debug.showTriRegions = false;
        }
        if (input.keyStates[DEBUG_SHOW_TRI_REGIONS].clicked) {
            debug.showTriRegions = !debug.showTriRegions;
            if (debug.showTriRegions)
                debug.showContours = false;
        }
        if (input.keyStates[DEBUG_SHOW_POLY_REGIONS].clicked) {
            debug.showNavMesh = !debug.showNavMesh;
            if (debug.showNavMesh)
                debug.showContours = false;
        }
        if (input.keyStates[DEBUG_SHOW_DUAL_MESH].clicked) {
            debug.showDual = !debug.showDual;
        }

        updateCameraPan(&camera, &input, &level, dt);

        Vec2 mousePos(input.mouseX / ScreenWidth - 0.5,
            input.mouseY / ScreenHeight - 0.5);

        Ray mouseRay = unproject(&viewCamera, mousePos);
        Vec3 groundPos = intersectGround0(mouseRay);

        if (input.keyStates[FIRE1].clicked) {
            findPath(&memoryArena, &navMesh, bot.entity.position, groundPos, &path);
            pullString(&memoryArena, &navMesh, bot.entity.position, groundPos, &path);
            updateBufferObject(pathVbo, path.points, path.length);
        }

        updateAIEntity(&bot, dt);
        updateEntity(&bot.entity, dt);

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

            /*
             * DRAW 3D COLOR VERTEX
             */
            Shader* flatDiffShader = &renderer.flatDiffShader;
            glUseProgram(flatDiffShader->progId);
            glUniformMatrix4fv(flatDiffShader->projLoc,
                1, true, viewCamera.projection.data);
            glUniformMatrix4fv(flatDiffShader->viewLoc,
                1, true, view.data);

            // Draw Ground
            glBindVertexArray(planeVao);

            Mat3 rot;
            identity(rot);
            glUniformMatrix3fv(flatDiffShader->rotLoc, 1, true, rot.data);
            glUniform3f(flatDiffShader->sizeLoc, MAP_WIDTH, MAP_HEIGHT, 0.0f);
            glUniform3f(flatDiffShader->posLoc, 0.5f * MAP_WIDTH, 0.5f * MAP_HEIGHT, 0.f);
            glUniform3f(flatDiffShader->diffuseLoc, .5f, .5f, .5f);

            glDrawElements(GL_TRIANGLES, 3 * planeMesh.fCount, GL_UNSIGNED_INT, 0);

            // Draw Walls
            glBindVertexArray(boxVao);

            for (uint32 j = 0; j < MAP_HEIGHT; j++) {
                for (uint32 i = 0; i < MAP_WIDTH; i++) {
                    uint32 val = level.tiles[i + j * MAP_HEIGHT];
                    if (val) {
                        Vec3 pos = Vec3(i + 0.5f, MAP_HEIGHT - j - 0.5f, val - 0.5);
                        drawBox(flatDiffShader, &boxMesh, pos, Vec3(0.75f, 0.75f, 1.f));
                    }
                }
            }
            drawBox(flatDiffShader, &boxMesh,
                bot.entity.position + Vec3(0.f, 0.f, .5f),
                Vec3(0.5f, 0.5f, 0.5f));

            drawBox(flatDiffShader, &boxMesh,
                groundPos, Vec3(.25f, .25f, 1.f));

            logOpenGLErrors();

#if 1
            {
                // RENDER DEBUG CODE HERE
                if (debug.showDistanceField || debug.showRegions) {
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
                    glUniform3f(texDiffShader->posLoc, 0.5f * MAP_WIDTH, 0.5f * MAP_HEIGHT, 0.1f);

                    glActiveTexture(GL_TEXTURE0);
                    glUniform1i(texDiffShader->diffTexLoc, 0);

                    if (debug.showDistanceField)
                        glBindTexture(GL_TEXTURE_2D, distanceField.texture.texId);
                    else
                        glBindTexture(GL_TEXTURE_2D, regionIds.texture.texId);

                    glDrawElements(GL_TRIANGLES, 3 * planeMesh.fCount, GL_UNSIGNED_INT, 0);
                }

                if (true || debug.showContours || debug.showTriRegions || debug.showNavMesh || debug.showDual) {

                    Shader* flatDiffShader = &renderer.flatColorShader;
                    glUseProgram(flatDiffShader->progId);
                    glUniformMatrix4fv(flatDiffShader->projLoc,
                        1, true, viewCamera.projection.data);
                    glUniformMatrix4fv(flatDiffShader->viewLoc,
                        1, true, view.data);

                    glUniformMatrix3fv(flatDiffShader->rotLoc, 1, true, rot.data);
                    glUniform3f(flatDiffShader->sizeLoc, 1, 1, 1);
                    glUniform3f(flatDiffShader->posLoc, 0, 0, 0);

                    if (debug.showContours) {
                        glPointSize(4.0f);
                        glUniform3f(flatDiffShader->diffuseLoc, 1.0f, 1.0f, 1.0f);

                        for (uint32 i = 0; i < contours.count; i++) {
                            glBindVertexArray(lineVaos[i]);
                            glDrawArrays(GL_POINTS, 0, contours.contours[i].count);
                        }

                        for (uint32 i = 0; i < contours.count; i++) {
                            glBindVertexArray(lineVaos[i]);
                            glDrawArrays(GL_LINE_LOOP, 0, contours.contours[i].count);
                        }
                    }

                    if (debug.showTriRegions) {
                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                        glUniform3f(flatDiffShader->diffuseLoc, 1.0f, 1.0f, 1.0f);

                        for (uint32 i = 0; i < contours.count; i++) {
                            glBindVertexArray(contourMeshVaos[i]);
                            glDrawElements(GL_TRIANGLES, 3 * triangulatedCountours[i].vCount, GL_UNSIGNED_INT, 0);
                        }
                        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                        glBindVertexArray(0);
                    }

                    logOpenGLErrors();
                    if (debug.showNavMesh) {
                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                        glUniform3f(flatDiffShader->diffuseLoc, .0f, .0f, 1.0f);

                        logOpenGLErrors();
                        for (uint32 i = 0; i < navMesh.polyCount; i++) {
                            uint32 iCount = polyVertCount(&navMesh, i);
                            glBindVertexArray(navVaos[i]);
                            glDrawElements(GL_LINE_LOOP, iCount, GL_UNSIGNED_INT, 0);
                        }
                        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    }

                    logOpenGLErrors();
                    if (debug.showDual) {
                        glBindVertexArray(dualVao);
                        glUniform3f(flatDiffShader->diffuseLoc, 1.0f, .0f, 0.0f);
                        glDrawElements(GL_LINES, dual.indCount, GL_UNSIGNED_INT, 0);
                    }
                }

                for (uint32 p = 0; p < path.polyPathLength; p++) {
                    uint32 iCount = polyVertCount(&navMesh, path.polys[p]);
                    glBindVertexArray(navVaos[path.polys[p]]);
                    glDrawElements(GL_LINE_LOOP, iCount, GL_UNSIGNED_INT, 0);
                    logOpenGLErrors();
                }

                if (path.length > 0) {
                    glBindVertexArray(pathVao);
                    glDrawArrays(GL_LINE_STRIP, 0, path.length);
                    logOpenGLErrors();
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
