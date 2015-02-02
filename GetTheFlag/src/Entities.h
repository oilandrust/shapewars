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
    
    // This Could be static (Better for cache?)
    Vec2 collisionSize;
    real32 acc;
    real32 drag;
    real32 spriteSize;
    real32 size;
    SDL_Surface* bitmaps[3];
    Texture textures[3];
    Texture* currentTexture;
};

void initializePlayer(Player* player);

void updatePlayer(Player* player, Input* input, Level* level, real32 dt);

#define MAX_BULLET_COUNT 50

struct Bullet {
    Vec2 position;
    Vec2 velocity;
};

struct BulletManager {
    Bullet bullets[MAX_BULLET_COUNT];
    uint32 bulletCount;
    
    // Constant among bullets
    Vec2 bulletSize;
    real32 bulletSpeed;
    Texture bulletTexture;
    SDL_Surface* bulletBitmap;
};

void initializeBullets(BulletManager* manager);

void updateBullets(BulletManager* manager, Level* level, real32 dt);

uint32 createBullet(BulletManager* manager);

void releaseBullet(BulletManager* manager,uint32 bulletIndex);

#endif