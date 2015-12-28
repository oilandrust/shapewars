#include <cstdio>

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include "Animation.h"
#include "Debug.h"
#include "Entities.h"
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

// TODO: 7 days
// Cleanup
// Bigger level
// Debug refactoring 2
// Fog of war
// Minimap
// Igui
// Start Screen
// Pause Menu
// Multiple entities

#define DEVENV

struct ViewCamera {
    Mat4 projection;
    Mat4 view;
    Vec3 position;
    Vec3 forward;
    Vec3 up;
    Vec3 right;
    real32 focalDistance;
    real32 aspect;
};

void viewCameraLookAt(ViewCamera* camera, const Vec3& position, const Vec3& target, const Vec3& up)
{
    camera->forward = normalize(target - position);
    camera->right = normalize(cross(camera->forward, up));
    camera->up = cross(camera->right, camera->forward);
    camera->position = position;

    lookAt(camera->view, camera->right, camera->up, camera->forward, camera->position);
}

struct Ray {
    Vec3 origin;
    Vec3 direction;
};

Vec3 intersectGround0(const Ray& ray)
{
    real32 t = -ray.origin.z / ray.direction.z;
    return ray.origin + t * ray.direction;
}

Ray unproject(ViewCamera* camera, const Vec2& screenPos)
{
    Ray result;
    result.origin = camera->position;

    Vec3 cameraMousePos = camera->aspect * screenPos.x * camera->right
        + screenPos.y * camera->up
        + camera->focalDistance * camera->forward;

    result.direction = normalize(cameraMousePos);

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

    Mesh3D boxMesh;
    createCube(&memoryArena, &boxMesh);
    GLuint boxVao = createIndexedVertexArray(&boxMesh);

    Mesh3D planeMesh;
    createPlane(&memoryArena, &planeMesh);
    GLuint planeVao = createIndexedVertexArray(&planeMesh);

    Debug debug;
    debug.planeVao = planeVao;
    debug.planeICount = 6;
    debug.planeSize = level.width;

    // Init the nav mesh.
    NavMesh navMesh;
    initializeNavMesh(&memoryArena, &debug, &level, &navMesh);
    debug.navMesh = &navMesh;

    CameraPan camera;
    initializeCameraPan(&camera, Vec2(level.width, level.height));
    camera.screenWidth = ScreenWidth;
    camera.screenHeight = ScreenHeight;

    ViewCamera viewCamera;
    real32 aspect = (real32)ScreenWidth / (real32)ScreenHeight;
    real32 fovy = 40.f;
    perspective(viewCamera.projection, fovy, aspect, 1.f, 200.f);
    viewCamera.focalDistance = 1.f / tanf(.5f * fovy * PI / 180.f);
    viewCamera.aspect = aspect;

    AIEntity bot;
    bot.entity.position = Vec3(1, 1, 0);

    Path path;
    initializePath(&memoryArena, &path, navMesh.polyCount);

    setAIEntityPath(&bot, &path);

    debug.path = &path;
    debug.pathVbo = createBufferObject(path.points, navMesh.polyCount + 2, GL_DYNAMIC_DRAW);
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

        if (input.keyStates[QUIT].clicked
            || input.keyStates[ESCAPE].clicked) {
            running = false;
        }

        // Update the input state
        processInput(&input);
        debugProcessInput(&debug, &input);

        // Reload the shader
        if (input.keyStates[DEBUG_RELOAD_SHADERS].clicked) {
            reloadShaders(&renderer);
        }

        updateCameraPan(&camera, &input, &level, dt);
        viewCameraLookAt(&viewCamera, camera.position, camera.target, Vec3(0, 0, 1));

        Vec2 mousePos(2 * (real32)input.mouseX / ScreenWidth - 1.f,
            1.f - 2 * (real32)input.mouseY / ScreenHeight);

        Ray mouseRay = unproject(&viewCamera, mousePos);
        Vec3 groundPos = intersectGround0(mouseRay);

        if (input.keyStates[FIRE1].clicked) {
            findPath(&memoryArena, &navMesh, bot.entity.position, groundPos, &path);
            pullString(&memoryArena, &navMesh, bot.entity.position, groundPos, &path);
            updateBufferObject(debug.pathVbo, path.points, path.length);
            debug.pathLength = path.length;

            setAIEntityPath(&bot, &path);
        }

        updateAIEntity(&bot, dt);
        updateEntity(&bot.entity, dt);

        Mat3 identity3;
        identity(identity3);

        Vec3 groundSize(level.width, level.height, 0.0f);
        Vec3 groundCenter(0.5f * level.width, 0.5f * level.height, 0.f);
        pushMeshPiece(&renderer, &renderer.flatDiffShader,
            planeVao, 3 * planeMesh.fCount,
            identity3, groundSize, groundCenter, Vec3(.5f));

        // Draw Walls
        Vec3 boxColor = Vec3(0.75);
        for (uint32 j = 0; j < level.width; j++) {
            for (uint32 i = 0; i < level.height; i++) {
                uint32 val = level.tiles[i + j * level.width];
                if (val) {
                    Vec3 boxSize = Vec3(0.75f, 0.75f, 1.f);
                    Vec3 pos = Vec3(i + 0.5f, level.height - j - 0.5f, val - 0.5);

                    pushMeshPiece(&renderer, &renderer.flatDiffShader,
                        boxVao, 3 * boxMesh.fCount,
                        identity3, boxSize, pos, boxColor);
                }
            }
        }

        // Bot box
        pushMeshPiece(&renderer, &renderer.flatDiffShader,
            boxVao, 3 * boxMesh.fCount,
            identity3, Vec3(0.5), bot.entity.position + Vec3(0.f, 0.f, .5f), boxColor);

        // Box target
        pushMeshPiece(&renderer, &renderer.flatDiffShader,
            boxVao, 3 * boxMesh.fCount,
            identity3, Vec3(.25f, .25f, 1.f), groundPos, boxColor);

        debugDraw(&debug, &renderer);

        // Set the view Matrix
        Mat4 view = viewCamera.view;
        inverseTransform(view);

        rendererBeginFrame(&renderer);

        renderAll(&renderer, viewCamera.projection, view);

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
