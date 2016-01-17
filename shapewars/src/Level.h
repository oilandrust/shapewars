#ifndef LEVELH
#define LEVELH

#include "ShapeWars.h"

#define MAP_WIDTH 32
#define MAP_HEIGHT 32

#define TEST_MAP_WIDTH 16
#define TEST_MAP_HEIGHT 16


extern uint8 level1[MAP_WIDTH * MAP_HEIGHT];
extern uint8 level2[MAP_WIDTH * MAP_HEIGHT];
extern uint8 level3[MAP_WIDTH * MAP_HEIGHT];
extern uint8 testlevel[TEST_MAP_WIDTH * TEST_MAP_HEIGHT];

struct Vec3;

struct Level {
    uint8* tiles;
    uint32 width;
    uint32 height;

    Vec3* walls;
    uint32 wallCount;
};

void initalizeLevel(MemoryArena* persistentArena, Level* level, uint8* levelTiles, uint32 width, uint32 height);

bool isWalkable(Level* level, const Vec3& pos);

#endif