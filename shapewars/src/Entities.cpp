#include "Entities.h"
#include "Input.h"
#include "Level.h"
#include "NavMeshQuery.h"

void initalizeAIEntity(AIEntity* aiEntity)
{
    aiEntity->currentTarget = 0;
}

void updateAIEntity(AIEntity* aiEntity, real32 dt)
{
    Path* path = &aiEntity->path;
    if (path->length == 0) {
        return;
    }

    Vec3 target = path->points[aiEntity->currentTarget];
    Vec3 pos = aiEntity->entity.position;

    Vec3 dir = target - pos;

    if (sqrLength(dir) < 0.1) {
        aiEntity->currentTarget++;
        if (aiEntity->currentTarget > path->length - 1) {
            path->length = 0;
            aiEntity->currentTarget = 0;
            aiEntity->entity.velocity = Vec3(0, 0, 0);
        }
    }
    else {
        aiEntity->entity.velocity = 5 * normalize(dir);
    }
}

void updateEntity(Entity* entity, real32 dt)
{
    entity->position += dt * entity->velocity;

    real32 vc = length(entity->velocity);
    if (vc > 0.0) {
        Vec3 forward = (1.f / vc) * entity->velocity;
        Vec3 up = Vec3(0, 0, 1);
        Vec3 right = cross(up, forward);
        fromFrame(entity->orientation, right, up, forward);
    }
}

/**
 * Camera
 */

void initializeCameraPan(CameraPan* camera, const Vec2& levelBounds)
{
    Vec3 targetToCamOffset(.0f, -0.9 * 30, 20);
    camera->position = camera->target + targetToCamOffset;

    camera->target = Vec3(0.5 * levelBounds.x, 0.3 * levelBounds.y, 0);

    camera->velocity = { 0, 0 };
    camera->accel = { 0, 0 };
    camera->drag = 10.0f;

    // acceleration and drag in m/s;
    camera->drag = 20.0f;
    camera->acc = 200.0f;
}

void updateCameraPan(CameraPan* camera, Input* input, Level* level, real32 dt)
{
    Vec3 accel = Vec3(0, 0, 0);
    if (input->keyStates[UP].held || input->mouseY == 0) {
        accel.y = 1.0f;
    }
    if (input->keyStates[DOWN].held || input->mouseY == camera->screenHeight - 1) {
        accel.y = -1.0f;
    }
    if (input->keyStates[LEFT].held || input->mouseX == 0) {
        accel.x = -1.0f;
    }
    if (input->keyStates[RIGHT].held || input->mouseX == camera->screenWidth - 1) {
        accel.x = 1.0f;
    }
    if (std::abs(accel.x) > 0.0f || std::abs(accel.y) > 0.0f) {
        accel = normalize(accel);
    }

    Vec3 acceleration = camera->acc * accel - camera->drag * camera->velocity;
    camera->velocity += acceleration * dt;

    camera->target = camera->target + camera->velocity * dt;
    camera->target = min(camera->target, Vec3(level->width, level->height, 0));
    camera->target = max(camera->target, Vec3(0, 0, 0));

    Vec3 targetToCamOffset(.0f, -10, 30);
    camera->position = camera->target + targetToCamOffset;
}
