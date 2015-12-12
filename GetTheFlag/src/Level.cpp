#include "Level.h"

#include <stb_image.h>

#include <SDL_surface.h>
#include <SDL_error.h>

/*
 * Static collision detection test of a rectangle to the walls in the level
 */
bool levelRectCollides(Level* level, const Rect& rect)
{
    // Find the zone containing the rect
    uint32 minTX = (uint32)rect.min.x;
    uint32 maxTX = (uint32)rect.max.x + 1;
    uint32 minTY = (uint32)(level->height - rect.max.y);
    uint32 maxTY = (uint32)(level->height - rect.min.y) + 1;

    Rect wallRect = {
        Vec2(0, 0),
        Vec2(1, 1)
    };

    for (uint32 j = minTY; j < maxTY; j++) {
        for (uint32 i = minTX; i < maxTX; i++) {
            if (levelValueAtTile(level, i, j) > 0) {
                wallRect.min = levelGridToWorld(level, i, j + 1);
                wallRect.max = levelGridToWorld(level, i + 1, j);
                if (rectCollidesRect(rect, wallRect)) {
                    return true;
                }
            }
        }
    }

    return false;
}

/*
 * Dynamic collision detection test of a rectangle to the walls in the level
 * Returns the closest intersection point along the trajectory and the normal at the contact edge
 * this use minkowski sum to simplify the test to a line/rect test
 */
bool levelRectCollides(Level* level,
    const Vec2& size, const Vec2& position,
    const Vec2& velocity, const real32 dt,
    Vec2& collisionPoint, Vec2& normal)
{
    if (!(dot(velocity, velocity) > 0.0f)) {
        return false;
    }

    Vec2 nPos = position + velocity * dt;

    Vec2 halfSize = 0.5f * size;
    // Find the zone containing the rect
    int32 minTX = min((int32)(position.x - halfSize.x), (int32)(nPos.x - halfSize.x));
    int32 maxTX = max((int32)(position.x + halfSize.x), (int32)(nPos.x + halfSize.x)) + 1;
    int32 minTY = min((int32)(level->height - position.y - halfSize.y), (int32)(level->height - nPos.y - halfSize.y));
    int32 maxTY = max((int32)(level->height - position.y + halfSize.y), (int32)(level->height - nPos.y + halfSize.y)) + 1;

    // Some values usefull for the computation of each rect
    real32 maxDist = length(velocity) * dt;
    Vec2 d = normalize(velocity);
    real32 fx = 1.0f / d.x;
    real32 fy = 1.0f / d.y;

    // Find the distance to the closest intersection
    // And the index of the box
    real32 tMin = 10000.0f;
    uint32 iC;
    uint32 jC;

    bool collides = false;
    for (int32 j = minTY; j < maxTY; j++) {
        for (int32 i = minTX; i < maxTX; i++) {
            if (levelValueAtTile(level, i, j) == 1) {
                // B rect, unit rect of the level grown by the test rect size for minkowski
                Vec2 bMin = levelToWorld(level, (real32)i - halfSize.x, (real32)j + halfSize.y + 1.0f);
                Vec2 bMax = levelToWorld(level, (real32)i + halfSize.x + 1.0f, (real32)j - halfSize.y);

                real32 txm = (bMin.x - position.x) * fx;
                real32 txM = (bMax.x - position.x) * fx;
                real32 tym = (bMin.y - position.y) * fy;
                real32 tyM = (bMax.y - position.y) * fy;

                real32 tmin = max(min(txm, txM), min(tym, tyM));
                real32 tmax = min(max(txm, txM), max(tym, tyM));

                //ASSERT(!(tmin!=tmin));
                //ASSERT(!(tmax!=tmax));

                if (tmin < tmax && tmax > 0) {
                    if (tmin < maxDist && tmin < tMin) {
                        collides = true;
                        tMin = tmin;
                        iC = i;
                        jC = j;
                    }
                }
            }
        }
    }

    if (collides) {
        ASSERT_MSG(!(tMin != tMin), "NAN in slab collision, fx or fy is inf");
        collisionPoint = position + tMin * d;

        // Compute the normal
        Vec2 bMin = levelToWorld(level, (real32)iC - halfSize.x, (real32)jC + halfSize.y + 1.0f);
        Vec2 bMax = levelToWorld(level, (real32)iC + halfSize.x + 1.0f, (real32)jC - halfSize.y);

        if (collisionPoint.x == bMin.x) {
            normal = Vec2(-1, 0);
        }
        else if (collisionPoint.x == bMax.x) {
            normal = Vec2(1, 0);
        }
        else if (collisionPoint.y == bMin.y) {
            normal = Vec2(0, -1);
        }
        else if (collisionPoint.y == bMax.y) {
            normal = Vec2(0, 1);
        }
        else {
            ASSERT_MSG(0, "Not Robust");
        }

        return true;
    }

    return false;
}
