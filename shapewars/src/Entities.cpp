#include "Entities.h"
#include "Input.h"
#include "Level.h"
#include "NavMeshQuery.h"

void setAIEntityPath(AIEntity* aiEntity, Path* path)
{
    aiEntity->path = path;
    aiEntity->currentTarget = 0;
}

void updateAIEntity(AIEntity* aiEntity, real32 dt)
{
    Path* path = aiEntity->path;
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
}

/*
 * PLAYER
 */

void initializePlayer(Player* player)
{
    // acceleration and drag in m/s;
    player->drag = 20.0f;
    player->acc = 200.0f;
    player->position = Vec3(10, 10, 0);
    player->velocity = Vec3(0, 0, 0);
    player->aimDir = Vec3(1, 0, 0);
    player->size = 3.0f;
    player->collisionSize = Vec2(0.5, 0.5);
}

void updatePlayer(Player* player, Input* input, Level* level, real32 dt)
{
    player->accel = Vec3(0, 0, 0);
    if (input->keyStates[UP].held) {
        player->accel.y = 1.0f;
    }
    if (input->keyStates[DOWN].held) {
        player->accel.y = -1.0f;
    }
    if (input->keyStates[LEFT].held) {
        player->accel.x = -1.0f;
    }
    if (input->keyStates[RIGHT].held) {
        player->accel.x = 1.0f;
    }
    if (std::abs(player->accel.x) > 0.0f || std::abs(player->accel.y) > 0.0f) {
        player->accel = normalize(player->accel);
        player->aimDir = player->accel;
    }
    Vec3 acceleration = player->acc * player->accel - player->drag * player->velocity;
    player->velocity += acceleration * dt;

    if (std::abs(player->velocity.x) > 0.0f || std::abs(player->velocity.y) > 0.0f) {
        Vec3 newPos = player->position + player->velocity * dt;
        Vec2 newPos2D = { newPos.x, newPos.y };
        Vec2 pos2D = { player->position.x, player->position.y };
        Vec2 velocity2D = { player->velocity.x, player->velocity.y };

        // Resolve collisions
        Vec2 collisionPoint;
        Vec2 normal;
        if (levelRectCollides(level,
                Vec2(player->collisionSize), pos2D, velocity2D, dt,
                collisionPoint, normal)) {
            // if we collide, we run the remaining distance along the wall
            real32 dtr;
            do {
                Vec2 dispInside = newPos2D - collisionPoint;
                Vec2 dispCorrected = dispInside - dot(normal, dispInside) * normal;

                if (length(dispCorrected) > 0) {
                    real32 oldVelocity = length(player->velocity);
                    dtr = dt - length(pos2D - collisionPoint) * oldVelocity;
                    velocity2D = oldVelocity * normalize(dispCorrected);
                    pos2D = collisionPoint + dispCorrected;
                }
                else {
                    pos2D = collisionPoint;
                    break;
                }
            } while (dtr > 0 && levelRectCollides(level,
                                    player->collisionSize, pos2D, velocity2D, dtr,
                                    collisionPoint, normal));

            if (levelRectCollides(level,
                    player->collisionSize, pos2D, velocity2D, dt,
                    collisionPoint, normal)) {
                pos2D = collisionPoint;
            }

            player->position = { pos2D, 0 };
            player->velocity = { velocity2D, 0 };
        }
        else {
            player->position = newPos;
        }

        // Stay inside the Level
        Vec3 size3 = Vec3(player->collisionSize.x, player->collisionSize.y, 0);
        player->position = max(player->position, 0.5f * size3);
        player->position = min(player->position, Vec3(level->width, level->height, 0) - 0.5f * size3);
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
