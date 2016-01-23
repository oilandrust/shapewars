#include "Entities.h"
#include "Input.h"
#include "Level.h"
#include "NavMeshQuery.h"
#include "Debug.h"

void startSteering(AIEntity* entity)
{
    entity->currentTarget = 0;
}

void stopSteering(AIEntity* entity)
{
    entity->currentTarget = NO_TARGET;
}

void updateAIEntity(AIEntity* aiEntity, real32 /* dt */)
{
    Path* path = &aiEntity->path;
    if (path->length == 0 || aiEntity->currentTarget == NO_TARGET) {
        aiEntity->entity.velocity = Vec3(0.f);
        return;
    }

    Vec3 target = path->points[aiEntity->currentTarget];
    Vec3 pos = aiEntity->entity.position;

    Vec3 dir = target - pos;

    if (sqrLength(dir) < 0.1f) {
        aiEntity->currentTarget++;
        if (aiEntity->currentTarget > path->length - 1) {
            path->length = 0;
            aiEntity->currentTarget = NO_TARGET;
            aiEntity->entity.velocity = Vec3(0.f, 0.f, 0.f);
        }
    }
    else {
        aiEntity->entity.velocity = 5.f * normalize(dir);
    }

	if (g_debug.showPath) {
		debugDrawLineStrip(&g_debugDraw, path->points, path->length);
	}
}

void updateEntity(Entity* entity, real32 dt)
{
    entity->position += dt * entity->velocity;

    real32 vc = length(entity->velocity);
    if (vc > 0.f) {
        Vec3 forward = (1.f / vc) * entity->velocity;
        Vec3 up = Vec3(0.f, 0.f, 1.f);
        Vec3 right = cross(up, forward);
        fromFrame(entity->orientation, right, up, forward);
    }
}

/**
 * Camera
 */

void initializeCameraPan(CameraPan* camera, const Vec2& levelBounds)
{
    Vec3 targetToCamOffset(.0f, -.9f * 30.f, 20.f);
    camera->position = camera->target + targetToCamOffset;

    camera->target = Vec3(0.5f * levelBounds.x, 0.3f * levelBounds.y, 0.f);

    camera->velocity = Vec3(0.f);
    camera->accel = Vec3(0.f);
    camera->drag = 10.0f;

    // acceleration and drag in m/s;
    camera->drag = 20.0f;
    camera->acc = 200.0f;
}

void updateCameraPan(CameraPan* camera, Input* input, Level* level, real32 dt)
{
    Vec3 accel = Vec3(0, 0, 0);
    if (input->keyStates[UP].held || input->mouseY <= 10) {
        accel.y = 1.0f;
    }
    if (input->keyStates[DOWN].held || input->mouseY >= camera->screenHeight - 10) {
        accel.y = -1.0f;
    }
    if (input->keyStates[LEFT].held || input->mouseX <= 10) {
        accel.x = -1.0f;
    }
    if (input->keyStates[RIGHT].held || input->mouseX >= camera->screenWidth - 10) {
        accel.x = 1.0f;
    }

    if (std::abs(accel.x) > 0.0f || std::abs(accel.y) > 0.0f) {
        accel = normalize(accel);
    }

    Vec3 acceleration = camera->acc * accel - camera->drag * camera->velocity;
    camera->velocity += acceleration * dt;

    camera->target = camera->target + camera->velocity * dt;
    camera->target = min3(camera->target, Vec3((real32)level->width, (real32)level->height, 0.f));
    camera->target = max3(camera->target, Vec3(0.f));

    Vec3 targetToCamOffset(.0f, -10.f, 30.f);
    camera->position = camera->target + targetToCamOffset;
}
