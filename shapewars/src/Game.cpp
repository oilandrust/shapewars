#include "Game.h"

#include "Debug.h"
#include "Input.h"
#include "Level.h"
#include "OpenGLClient.h"
#include "Renderer.h"
#include "ShapeWars.h"

static void viewCameraLookAt(ViewCamera* camera, const Vec3& position, const Vec3& target, const Vec3& up)
{
    camera->forward = normalize(target - position);
    camera->right = normalize(cross(camera->forward, up));
    camera->up = cross(camera->right, camera->forward);
    camera->position = position;

    lookAt(camera->view, camera->right, camera->up, camera->forward, camera->position);
}

static Vec3 intersectGround0(const Ray& ray)
{
    real32 t = -ray.origin.z / ray.direction.z;
    return ray.origin + t * ray.direction;
}

static Ray unproject(ViewCamera* camera, const Vec2& screenPos)
{
    Ray result;
    result.origin = camera->position;

    Vec3 cameraMousePos = camera->aspect * screenPos.x * camera->right
        + screenPos.y * camera->up
        + camera->focalDistance * camera->forward;

    result.direction = normalize(cameraMousePos);

    return result;
}

void setWindowSize(Game* game, uint32 width, uint32 height)
{
    game->screenSize = Vec2((real32)width, (real32)height);

    CameraPan* camera = &game->camera;
    ViewCamera* viewCamera = &game->viewCamera;

    camera->screenWidth = game->screenSize.x;
    camera->screenHeight = game->screenSize.y;

    real32 aspect = (real32)game->screenSize.x / (real32)game->screenSize.y;
    real32 fovy = 40.f;
    memset(viewCamera->projection.data, 0, 16 * sizeof(real32));
    perspective(viewCamera->projection, fovy, aspect, 1.f, 200.f);
    viewCamera->focalDistance = 1.f / tanf(.5f * fovy * PI / 180.f);
    viewCamera->aspect = aspect;
}

void initializeGame(Game* game)
{
    CameraPan* camera = &game->camera;
    Level* level = game->level;

    initializeCameraPan(camera, Vec2(level->width, level->height));

    game->bot.entity.position = Vec3(3.f, 3.f, 0.f);
    identity(game->bot.entity.orientation);
}

void handleInputAndUpdateGame(Game* game, Input* input, real32 dt)
{
    // Update Camera
    CameraPan* camera = &game->camera;
    ViewCamera* viewCamera = &game->viewCamera;

    updateCameraPan(camera, input, game->level, dt);
    viewCameraLookAt(viewCamera, camera->position, camera->target, Vec3(0, 0, 1));

    AIEntity* bot = &game->bot;
    if (input->keyStates[MOUSE_RIGHT].clicked || input->keyStates[FIRE1].clicked) {
        // Cast a ray against the ground
        const Vec2& screenSize = game->screenSize;
        Vec2 mousePos(2 * (real32)input->mouseX / screenSize.x - 1.f, 1.f - 2 * (real32)input->mouseY / screenSize.y);

        Ray mouseRay = unproject(viewCamera, mousePos);
        Vec3 groundPos = intersectGround0(mouseRay);

        // If the clicked position is on a walkable area, run pathfinding.
        if (isWalkable(game->level, groundPos)) {
            MemoryArena* tempArena = game->tempArena;
            NavMesh* navMesh = game->navMesh;
            Path* path = &bot->path;

            if (findPath(tempArena, navMesh, bot->entity.position, groundPos, path)) {
                pullString(tempArena, navMesh, bot->entity.position, groundPos, path);

                startSteering(bot);
            }
            else {
                stopSteering(bot);
            }
        }
    }

    updateAIEntity(bot, dt);
    updateEntity(&bot->entity, dt);
}

void renderGame(Game* game, Renderer* renderer)
{
    Mat3 identity3;
    identity(identity3);

    Level* level = game->level;

    Vec3 groundSize((real32)level->width, (real32)level->height, 0.0f);
    Vec3 groundCenter(0.5f * level->width, 0.5f * level->height, 0.f);

    pushPlanePiece(renderer, &renderer->groundShader, identity3, groundSize, groundCenter, Vec3(.70f));

    Vec3 boxColor = Vec3(0.8f);
    // Draw Walls
    if (g_debug.showWalls) {
        Vec3 boxSize = Vec3(1.f);
        Vec3* walls = level->walls;
        uint32 wallCount = level->wallCount;

        for (uint32 i = 0; i < wallCount; i++) {
            pushBoxPiece(renderer, &renderer->wallShader, identity3, boxSize, walls[i], boxColor);
        }
    }
    // Bot box
    real32 r = max(0.5f, *g_debug.agentRadius);
    Vec3 botPos = game->bot.entity.position + Vec3(0.f, 0.f, r);
    pushBoxPiece(renderer, &renderer->flatDiffShader, game->bot.entity.orientation, Vec3(r), botPos, boxColor);
}
