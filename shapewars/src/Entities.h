#ifndef ENTITIESH
#define ENTITIESH

#include "OpenGLClient.h"
#include "Vec2.h"

struct Input;
struct Level;
struct Path;

struct Entity {
    Vec3 position;
    Vec3 velocity;
};

struct AIEntity {
    Entity entity;
    Path* path;
    uint32 currentTarget;
};

void updateAIEntity(AIEntity* entity, real32 dt);

struct NavMesh;

void setAIEntityPath(AIEntity* aiEntity, Path* path);

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

    int32 screenWidth;
    int32 screenHeight;

    // Physics constants.
    real32 drag;
    real32 acc;
};

void initializeCameraPan(CameraPan* camera, const Vec2& levelBounds);

void updateCameraPan(CameraPan* camera, Input* input, Level* level, real32 dt);

#endif