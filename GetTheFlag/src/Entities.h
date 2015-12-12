#ifndef ENTITIESH
#define ENTITIESH

#include "Vec2.h"
#include "OpenGLClient.h"

struct Input;
struct Level;


struct Entity {
    Vec3 position;
    Vec3 velocity;
};

struct AIEntity {
    Vec3 target;
    Entity entity;
    
    uint32 path[512];
    uint32 pathLength;
};

void updateAIEntity(AIEntity* entity);

void setAIEntityTarget(Level* level, AIEntity* aiEntity, const Vec3& target);

void updateEntity(Entity* entity);


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