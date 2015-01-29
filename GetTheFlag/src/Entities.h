#ifndef ENTITIESH
#define ENTITIESH

#include "Vec2.h"
#include "OpenGLClient.h"

struct Input;
struct Level;

struct Player {
    Vec2 position;
    Vec2 velocity;
    Vec2 accel;
    Vec2 aimDir;
    real32 acc;
    real32 drag;
    real32 spriteSize;
    real32 size;
    Vec2 collisionSize;
    SDL_Surface* bitmaps[3];
    Texture textures[3];
    Texture* currentTexture;
};


void updatePlayer(Player* player, Input* input, Level* level, real32 dt);

#define MAX_BULLET_COUNT 50

struct Bullet {
    Vec2 position;
    Vec2 velocity;
};

struct BulletManager {
    Bullet* bullets;
    uint32 bulletCount;
    real32 bulletSpeed;
    Vec2 bulletSize;
    SDL_Surface* bulletBitmap;
};

void updateBullets(BulletManager* manager, Level* level, real32 dt);

uint32 createBullet(BulletManager* manager);

void releaseBullet(BulletManager* manager,uint32 bulletIndex);

#endif