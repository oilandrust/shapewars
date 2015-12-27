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
    intializeRenderer(&memoryArena, &renderer);

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

        {

            Mat3 identity3;
            identity(identity3);

            Vec3 groundSize(MAP_WIDTH, MAP_HEIGHT, 0.0f);
            Vec3 groundCenter(0.5f * MAP_WIDTH, 0.5f * MAP_HEIGHT, 0.f);
            pushMeshPiece(&renderer, &renderer.flatDiffShader,
                planeVao, 3 * planeMesh.fCount,
                identity3, groundSize, groundCenter, Vec3(.5f));

            // Draw Walls
            Vec3 boxColor = Vec3(0.75);

            for (uint32 j = 0; j < MAP_HEIGHT; j++) {
                for (uint32 i = 0; i < MAP_WIDTH; i++) {
                    uint32 val = level.tiles[i + j * MAP_HEIGHT];
                    if (val) {
                        Vec3 boxSize = Vec3(0.75f, 0.75f, 1.f);
                        Vec3 pos = Vec3(i + 0.5f, MAP_HEIGHT - j - 0.5f, val - 0.5);

                        pushMeshPiece(&renderer, &renderer.flatDiffShader,
                            boxVao, 3 * boxMesh.fCount,
                            identity3, boxSize, pos, boxColor);
                    }
                }
            }
            pushMeshPiece(&renderer, &renderer.flatDiffShader,
                boxVao, 3 * boxMesh.fCount,
                identity3, Vec3(0.5), bot.entity.position + Vec3(0.f, 0.f, .5f), boxColor);

            pushMeshPiece(&renderer, &renderer.flatDiffShader,
                boxVao, 3 * boxMesh.fCount,
                identity3, Vec3(.25f, .25f, 1.f), groundPos, boxColor);

            // Set the view Matrix
            identity(viewCamera.view);
            lookAt(viewCamera.view, camera.position, camera.target, Vec3(0, 0, 1));

            Mat4 view = viewCamera.view;
            inverseTransform(view);

            // Render Begin
            rendererBeginFrame(&renderer);

            logOpenGLErrors();

            // RENDER DEBUG CODE HERE
            if (debug.showDistanceField || debug.showRegions) {
                if (debug.showDistanceField) {
                    pushMeshPieceTextured(&renderer, &renderer.texDiffShader, planeVao, 3 * planeMesh.fCount,
                        distanceField.texture.texId, identity3, groundSize, groundCenter);
                }
                else {
                    pushMeshPieceTextured(&renderer, &renderer.texDiffShader, planeVao, 3 * planeMesh.fCount,
                        regionIds.texture.texId, identity3, groundSize, groundCenter);
                }
            }

            if (true || debug.showContours || debug.showTriRegions || debug.showNavMesh || debug.showDual) {
                if (debug.showContours) {
                    for (uint32 i = 0; i < contours.count; i++) {
                        pushArrayPiece(&renderer, &renderer.flatColorShader,
                            lineVaos[i], contours.contours[i].count, ARRAY_POINTS,
                            identity3, Vec3(1.f), Vec3(0.f), Vec3(1.f));
                    }
                    for (uint32 i = 0; i < contours.count; i++) {
                        pushArrayPiece(&renderer, &renderer.flatColorShader,
                            lineVaos[i], contours.contours[i].count, ARRAY_LINE_LOOP,
                            identity3, Vec3(1.f), Vec3(0.f), Vec3(1.f));
                    }
                }

                if (debug.showTriRegions) {
                    for (uint32 i = 0; i < contours.count; i++) {
                        pushMeshPieceWireframe(&renderer, &renderer.flatColorShader,
                            contourMeshVaos[i], 3 * triangulatedCountours[i].vCount,
                            identity3, Vec3(1.0f), Vec3(0.f), Vec3(1.0f));
                    }
                }
                if (debug.showNavMesh) {
                    for (uint32 i = 0; i < navMesh.polyCount; i++) {
                        uint32 iCount = polyVertCount(&navMesh, i);
                        pushIndexedArrayPiece(&renderer, &renderer.flatColorShader, navVaos[i], iCount, INDEXED_ARRAY_LINE_LOOP,
                            identity3, Vec3(1.0f), Vec3(.0f), Vec3(0, 0, 1));
                    }
                }
                if (debug.showDual) {
                    pushIndexedArrayPiece(&renderer, &renderer.flatColorShader, dualVao, dual.indCount, INDEXED_ARRAY_LINES,
                        identity3, Vec3(1.0f), Vec3(.0f), Vec3(1, 0, 0));
                }
            }

            for (uint32 p = 0; p < path.polyPathLength; p++) {
                uint32 iCount = polyVertCount(&navMesh, path.polys[p]);
                pushIndexedArrayPiece(&renderer, &renderer.flatColorShader, navVaos[path.polys[p]], iCount, INDEXED_ARRAY_LINE_LOOP,
                    identity3, Vec3(1.0f), Vec3(.0f), Vec3(1, 0, 0));
            }
            if (path.length > 0) {
                pushArrayPiece(&renderer, &renderer.flatColorShader,
                    pathVao, path.length, ARRAY_LINE_STRIP,
                    identity3, Vec3(1.f), Vec3(0.f), Vec3(1.f));
            }

            renderAll(&renderer, viewCamera.projection, view);

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
