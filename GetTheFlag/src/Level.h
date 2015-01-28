#ifndef LEVELH
#define LEVELH

#include "GetTheFlag.h"
#include "Vec2.h"

enum TILE_TYPE {
    WALL = 0,
    MACHINE_GUN,
    SHOTGUN,
    BOMB,
    HEART,
    MAX_ENTITY_TYPE
};



struct Level {
    uint8* tiles;
    uint32 width;
    uint32 height;
    real32 pixelPerUnit;
};

inline uint8 levelValueAtTile(Level* level, uint32 x, uint32 y)
{
    return level->tiles[x+y*level->width];
}

inline uint8 levelValueAt(Level* level, const Vec2& pos)
{
    uint32 tileX = (uint32)pos.x;
    uint32 tileY = (uint32)(level->height - pos.y);
    return levelValueAtTile(level, tileX, tileY);
}

inline Vec2 levelGridToWorld(Level* level, uint32 i, uint32 j)
{
    return Vec2(i, level->height-j);
}

inline Vec2 levelToWorld(Level* level, real32 i, real32 j)
{
    return Vec2(i, level->height-j);
}

inline bool isOutsideLevel(real32 w, real32 h, const Vec2& v)
{
    return v.x < 0 || v.y < 0 || v.x > w || v.y > h;
}

struct Rect {
    Vec2 min;
    Vec2 max;
};

inline bool rectCollidesRect(const Rect& a, const Rect& b)
{
    return (max(a.min.x, b.min.x) < min(a.max.x, b.max.x))
    && (max(a.min.y, b.min.y) < min(a.max.y, b.max.y));
}

extern bool levelRectCollides(Level* level, const Rect& rect);

bool levelRectCollides(Level* level, const Vec2& size, const Vec2& position, const Vec2& velocity, const real32 dt, Vec2& collisionPoint, Vec2& normal );

// TODO: Free tiles memory
bool loadLevel(Level* level, const char* filename);

#endif