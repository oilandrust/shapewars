#include <cstdio>

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include "Animation.h"
#include "Debug.h"
#include "Entities.h"
#include "Game.h"
#include "Input.h"
#include "Level.h"
#include "Mat3.h"
#include "Mat4.h"
#include "Mesh.h"
#include "NavMeshGen.h"
#include "NavMeshQuery.h"
#include "OpenGLClient.h"
#include "Renderer.h"
#include "ShapeWars.h"
#include "Vec3.h"

// TODO: 2 days
// Cleanup
// Antializasing
// Wall edge rendering
// shadow mapping
// temporary arena
// Bigger level
// Debug refactoring 2
// Fog of war
// Minimap
// Igui
// Start Screen
// Pause Menu
// Multiple entities

#define DEVENV

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

    Memory memory;
    size_t persistentMemSize = Megabytes(512);
    // Memory used to store data that have a long lifetime, ex: textures, navigation mesh data, ...
    initializeArena(&memory.persistentArena, new uint8[persistentMemSize], persistentMemSize);
    // Memory used for temporary usage like allocating an array for an algo, ...
    initializeArena(&memory.temporaryArena, new uint8[persistentMemSize], persistentMemSize);

    // Initialize the rendering resources
    Renderer renderer;
    intializeRenderer(&memory.persistentArena, &renderer);

    // Load The Level
    Level level;
    level.width = MAP_WIDTH;
    level.height = MAP_HEIGHT;
    level.tiles = level2;

    // The cube walls.
    Vec3* walls = pushArray<Vec3>(&memory.persistentArena, level.width * level.height);
    uint32 wallCount = 0;
    for (uint32 j = 0; j < level.width; j++) {
        for (uint32 i = 0; i < level.height; i++) {
            uint32 val = level.tiles[i + j * level.width];
            if (val) {
                walls[wallCount++] = Vec3(i + 0.5f, level.height - j - 0.5f, val - 0.5);
            }
        }
    }
    popArray<Vec3>(&memory.persistentArena, level.width * level.height - wallCount);
    level.walls = walls;
    level.wallCount = wallCount;

    Game game;
    game.tempArena = &memory.temporaryArena;
    game.level = &level;
    game.screenSize = Vec2(ScreenWidth, ScreenHeight);

    Debug debug;
    debug.planeSize = level.width;
    game.debug = &debug;

    // Init the nav mesh.
    NavMesh navMesh;
    initializeNavMesh(&memory, &debug, &level, &navMesh, 1 * level.width, 1 * level.height);
    resetArena(&memory.temporaryArena);
    debug.navMesh = &navMesh;
    game.navMesh = &navMesh;

    initializeGame(&game);

    initializePath(&memory.persistentArena, &game.bot.path, navMesh.polyCount);

    debug.path = &game.bot.path;
    debug.pathVbo = createBufferObject(game.bot.path.points, navMesh.polyCount + 2, GL_DYNAMIC_DRAW);
    debug.pathVao = createVertexArray(debug.pathVbo);

    Input input;
    memset(&input, 0, sizeof(input));

    // Timers
    uint64 lastCounter = SDL_GetPerformanceCounter();
    uint64 endCounter = 0;
    uint64 counterFrequency = SDL_GetPerformanceFrequency();

    // Check error at initialization
    ASSERT_MSG(!glGetError(), "OpenGl Error after initialization");

    bool running = true;
    // The main Loop
    while (running) {

        // Time delta in seconds
        real32 dt = targetMsPerFrame / 1000.0f;

        // Update the input state
        processInput(&input);

        // Exit
        if (input.keyStates[QUIT].clicked
            || input.keyStates[ESCAPE].clicked) {
            running = false;
        }
        // Reload the shader
        if (input.keyStates[DEBUG_RELOAD_SHADERS].clicked) {
            reloadShaders(&renderer);
        }

        // Update
        handleInputAndUpdateGame(&game, &input, dt);
        debugHandleInput(&debug, &input);

        // Draw
        renderGame(&game, &renderer);
        renderDebug(&debug, &renderer);

        // Set the view Matrix
        Mat4 view = game.viewCamera.view;
        inverseTransform(view);

        rendererBeginFrame(&renderer);
        renderAll(&renderer, game.viewCamera.projection, view);

        SDL_GL_SwapWindow(window);
        logOpenGLErrors();

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
        lastCounter = endCounter;
    }

    // Cleanup
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}
