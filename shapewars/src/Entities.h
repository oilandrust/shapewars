#ifndef ENTITIESH
#define ENTITIESH

#include "Mat3.h"
#include "NavMeshQuery.h"
#include "Vec2.h"

struct Input;
struct Level;

struct Entity {
    Vec3 position;
    Vec3 velocity;
    Mat3 orientation;
};

#define NO_TARGET 0xffffffff

struct AIEntity {
    Entity entity;
    Path path;
    uint32 currentTarget;
};

void startSteering(AIEntity* entity);

void stopSteering(AIEntity* entity);

void updateAIEntity(AIEntity* entity, real32 dt);

void updateEntity(Entity* entity, real32 dt);

struct Entities {
    Entity bots[128];
    uint32 nBots;
};

struct CameraPan {
    Vec3 position;
    Vec3 target;
    Vec3 velocity;
    Vec3 accel;

    real32 screenWidth;
    real32 screenHeight;

    // Physics constants.
    real32 drag;
    real32 acc;
};

void initializeCameraPan(CameraPan* camera, const Vec2& levelBounds);

void updateCameraPan(CameraPan* camera, Input* input, Level* level, real32 dt);

#endif