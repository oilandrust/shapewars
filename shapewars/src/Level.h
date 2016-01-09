#ifndef LEVELH
#define LEVELH

#include "ShapeWars.h"

#define MAP_WIDTH 32
#define MAP_HEIGHT 32

extern uint8 level1[MAP_WIDTH * MAP_HEIGHT];
extern uint8 level2[MAP_WIDTH * MAP_HEIGHT];

struct Vec3;

struct Level {
    uint8* tiles;
    uint32 width;
    uint32 height;

    Vec3* walls;
    uint32 wallCount;
};

bool isWalkable(Level* level, const Vec3& pos);

#endif