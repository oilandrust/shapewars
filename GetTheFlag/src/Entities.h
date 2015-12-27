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

    // Physics constants.
    real32 drag;
    real32 acc;
};

struct Player {
    Vec3 position;
    Vec3 velocity;
    Vec3 accel;
    Vec3 aimDir;

    Vec2 collisionSize;
    real32 acc;
    real32 drag;
    real32 spriteSize;
    real32 size;
};

void initializePlayer(Player* player);

void updatePlayer(Player* player, Input* input, Level* level, real32 dt);

void initializeCameraPan(CameraPan* camera);

void updateCameraPan(CameraPan* camera, Input* input, Level* level, real32 dt);

#endif