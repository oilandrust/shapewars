#include <cstdio>

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#undef main

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
#include "Text.h"
#include "Vec3.h"

#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef _WIN32
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
#endif

// TODO:
// Fix full screen toggle
// Memory alignment
// Antializasing
// Wall edge rendering
// shadow mapping
// Bigger level
// Debug refactoring 2
// Fog of war
// Minimap
// Igui
// Start Screen
// Pause Menu
// Multiple entities

SDL_Window* createSDLGLWindow(uint32& width, uint32& height, bool hd, bool fullScreen)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        return 0;
    }

    SDL_DisplayMode current;
    for (int32 i = 0; i < SDL_GetNumVideoDisplays(); i++) {
        int32 ret = SDL_GetCurrentDisplayMode(i, &current);
        ASSERT(ret == 0);
        if (ret != 0)
            SDL_Log("Could not get display mode for video display #%d: %s", i, SDL_GetError());
        else
            SDL_Log("Display #%d: current display mode is %dx%dpx @ %dhz. \n", i, current.w, current.h, current.refresh_rate);
    }

    if (hd) {
        width = current.w;
        height = current.h;
    }

    uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;
    if (fullScreen) {
        windowFlags |= SDL_WINDOW_FULLSCREEN;
    }

    SDL_Window* window = SDL_CreateWindow("Box War", 0, 0, width, height, windowFlags);

    if (!window) {
        return 0;
    }

    SDL_Surface* window_surface = 0;

    window_surface = SDL_GetWindowSurface(window);
    ASSERT_MSG(window_surface, "SDL_error %s", SDL_GetError());
    if (!window_surface) {
        return 0;
    }

    // Initialze OpenGL 3.1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    //Create context
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    ASSERT_MSG(!glGetError(), "Error Creating Contex");
    ASSERT_MSG(glContext, "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
    if (!glContext) {
        return 0;
    }

    //Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum result = glewInit();
    ASSERT_MSG(result == GLEW_OK, "Error initializing GLEW! %s", glewGetErrorString(result));
    glGetError();

    //Use Vsync
    if (SDL_GL_SetSwapInterval(1) < 0) {
        printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
    }

    return window;
}

void generateLandscape(MemoryArena* tempArena, real32 width, real32 height, GLuint& vao, uint32& count)
{
    uint32 cubeCount = 300;
    Vec3* pBuffer = pushArray<Vec3>(tempArena, 24 * cubeCount);
    Vec3* nBuffer = pushArray<Vec3>(tempArena, 24 * cubeCount);
    uint32* iBuffer = pushArray<uint32>(tempArena, 36 * cubeCount);

    uint32 vCount = 0;
    uint32 iCount = 0;

    real32 borderX = 0.5f * width;
    real32 borderY = 0.5f* height;

    real32 bMinSize = 1.f;
    real32 bMaxSize = 6.f;

    uint32 currenCount = 0;
    // Draw random boxes outside the level
    while (currenCount < cubeCount) {
        Vec3 c(randRangeReal32(-borderX, width + borderX), randRangeReal32(-borderY, height + borderY), 0.f);
        if (c.x >= 0.f && c.x <= width && c.y >= 0.f && c.y <= height) {
            continue;
        }
        Vec3 s(randRangeReal32(bMinSize, bMaxSize), randRangeReal32(bMinSize, bMaxSize), randRangeReal32(bMinSize, bMaxSize));
        // Clip with the level bounds.
        if (c.x < .0f && c.x + 0.5f * s.x > .0f) {
            c.x = -0.5f * s.x;
        }
        if (c.x > width && c.x - 0.5f * s.x < width) {
            c.x = width + 0.5f * s.x;
        }
        if (c.y < 0 && c.y + 0.5f * s.y > .0f) {
            c.y = -0.5f * s.y;
        }
        if (c.y > height && c.y - 0.5f * s.y < height) {
            c.y = height + 0.5f * s.y;
        }
        pushBox(pBuffer, nBuffer, iBuffer, c, s, vCount, iCount);
        currenCount++;
    }

    Mesh3D mesh;
    memset(&mesh, 0, sizeof(Mesh3D));
    mesh.positions = pBuffer;
    mesh.normals = nBuffer;
    mesh.indices = iBuffer;
    mesh.vCount = vCount;
    mesh.fCount = 12 * cubeCount;

    vao = createIndexedVertexArray(&mesh);
    count = iCount;

    resetArena(tempArena);
}

void initializePathFinding(Memory* memory, size_t memBegin, Debug* debug, Game* game, Level* level, NavMesh* navMesh, real32 agentRadius) {
	resetArena(&memory->persistentArena, memBegin);
	
	memset(navMesh, 0, sizeof(navMesh));
	initializeNavMesh(memory, debug, level, navMesh, 2 * level->width, 2 * level->height, agentRadius);
	resetArena(&memory->temporaryArena);
	
	initializePath(&memory->persistentArena, &game->bot.path, navMesh->polyCount);
}

int main()
{
	char cCurrentPath[FILENAME_MAX];
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) {
		return errno;
	}
	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
	printf ("Working directory is %s", cCurrentPath);

    uint32 ScreenWidth = 640;
    uint32 ScreenHeight = 480;
    bool fullScreen = false;
    bool hd = false;

    SDL_Window* window = createSDLGLWindow(ScreenWidth, ScreenHeight, hd, fullScreen);
    if (window == NULL) {
        return -1;
    }

    int32 monitorRefreshRate = 60;
    int32 gameRefreshRate = monitorRefreshRate;
    real32 targetMsPerFrame = 1000.0f / gameRefreshRate;

    Memory memory;
    size_t persistentMemSize = Megabytes(512);
    // Memory used to store data that have a long lifetime, ex: textures, navigation mesh data, ...
    initializeArena(&memory.persistentArena, new uint8[persistentMemSize], persistentMemSize);
    // Memory used for temporary usage like allocating an array for an algo, ...
    initializeArena(&memory.temporaryArena, new uint8[persistentMemSize], persistentMemSize);

    // Initialize the rendering resources
    Renderer renderer;
	memset(&renderer, 0, sizeof(Renderer));
    intializeRenderer(&memory.persistentArena, &renderer);

    TextRenderer textRenderer;
    initalizeTextRenderer(&memory, &textRenderer);
    textRenderer.shader = &renderer.textShader;
    textRenderer.screenRes = Vec2(ScreenWidth, ScreenHeight);

    // Level
    Level level;
    initalizeLevel(&memory.persistentArena, &level, level3, MAP_WIDTH, MAP_HEIGHT);

    GLuint landscapeVao;
    uint32 landscapeCount = 0;
    generateLandscape(&memory.temporaryArena, (real32)level.width, (real32)level.height, landscapeVao, landscapeCount);

    Game game;
    game.tempArena = &memory.temporaryArena;
    game.level = &level;
    game.screenSize = Vec2(ScreenWidth, ScreenHeight);


	real32 agentRadius = 0.3f;
	real32 oldAgentRadius = agentRadius;

	Debug* debug = &g_debug;
 	memset(&g_debug, 0, sizeof(Debug));
	debug->agentRadius = &agentRadius;
	debug->planeSize = (real32)level.width;
	debug->font = &textRenderer.defaultFont;
    game.debug = debug;
	initalizeDebug(debug);

	DebugDraw* debugDraw = &g_debugDraw;
	initializeDebugDraw(debugDraw);

    NavMesh navMesh;
	debug->navMesh = &navMesh;
	game.navMesh = &navMesh;
	initializeGame(&game);

	size_t navMemBegin = memory.persistentArena.used;

	initializePathFinding(&memory, navMemBegin, debug, &game, &level, &navMesh, agentRadius);

	Input input;
    memset(&input, 0, sizeof(input));

    // Timers
    uint64 lastCounter = SDL_GetPerformanceCounter();
    uint64 endCounter = 0;
    uint64 counterFrequency = SDL_GetPerformanceFrequency();
    real32 fps = 0.f;

    // Check error at initialization
    ASSERT_MSG(!glGetError(), "OpenGl Error after initialization");

    bool running = true;
    // The main Loop
    while (running) {

        // Time delta in seconds
        real32 dt = targetMsPerFrame / 1000.0f;
        debug->fps = fps;

        // Update the input state
        processInput(&input);

        // Exit
        if (input.keyStates[QUIT].clicked
            || input.keyStates[ESCAPE].clicked) {
            running = false;
        }
        // Reload the shaders
        if (input.keyStates[DEBUG_RELOAD_SHADERS].clicked) {
            reloadShaders(&renderer);
        }
        // Toggle full-screen
        if (input.keyStates[DEBUG_TOGGLE_FULLSCREEN].clicked) {
			if (fullScreen) {
               int32 res = SDL_SetWindowFullscreen(window, 0);
               ASSERT(res);
               fullScreen = false;
			}
			else {
				int32 res = SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
				ASSERT(res);
				fullScreen = true;
           }
		}
		// Change agent radius.
		if (agentRadius != oldAgentRadius) {
			if (agentRadius < 0.f) {
				agentRadius = 0.f;
			}
			oldAgentRadius = agentRadius;
			initializePathFinding(&memory, navMemBegin, debug, &game, &level, &navMesh, agentRadius);
		}
		
        // Update
        handleInputAndUpdateGame(&game, &input, dt);
        debugHandleInput(debug, &input);

        Mat3 rot;
        identity(rot);
        pushPlanePiece(&renderer, &renderer.flatDiffShader, rot,
            Vec3(2.5f * level.width, 2.5f * level.height, 1.f), Vec3(0.5f * level.width, 0.5f * level.height, -.1f), Vec3(0.6f));
        pushMeshPiece(&renderer, &renderer.flatDiffShader, landscapeVao, landscapeCount, rot, Vec3(1.f), Vec3(0.f), Vec3(0.7f));

        // Draw
        renderGame(&game, &renderer);

        renderDebug(debug, &renderer, &textRenderer);

        // Set the view Matrix
        Mat4 view = game.viewCamera.view;
        inverseTransform(view);

        // Render
        rendererBeginFrame(&renderer);
		renderAll(&renderer, game.viewCamera.projection, view);
		renderText(&textRenderer);

        SDL_GL_SwapWindow(window);

        // Update the clock
        endCounter = SDL_GetPerformanceCounter();
        uint64 couterElapsed = endCounter - lastCounter;
        real32 msElapsed = ((1000.0f * (real32)couterElapsed) / (real32)counterFrequency);

        // TODO COMPRESS THIS
        if (msElapsed < targetMsPerFrame) {
            SDL_Delay((uint32)targetMsPerFrame - (uint32)msElapsed);
            lastCounter = endCounter;
            endCounter = SDL_GetPerformanceCounter();
            couterElapsed = endCounter - lastCounter;
            msElapsed += ((1000.0f * (real32)couterElapsed) / (real32)counterFrequency);
        }
        fps = 1000.f / msElapsed;
        lastCounter = endCounter;

		logOpenGLErrors();
    }

    // Cleanup
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}
