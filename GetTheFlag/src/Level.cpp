#include "Level.h"

#include <SDL_surface.h>
#include <SDL_error.h>

/*
 * Static collision detection test of a rectangle to the walls in the level
 */
bool levelRectCollides(Level* level, const Rect& rect)
{
    // Find the zone containing the rect
    uint32 minTX = (uint32)rect.min.x;
    uint32 maxTX = (uint32)rect.max.x+1;
    uint32 minTY = (uint32)(level->height - rect.max.y);
    uint32 maxTY = (uint32)(level->height - rect.min.y)+1;
    
    Rect wallRect = {
        Vec2(0,0),
        Vec2(1,1)
    };
    
    for (uint32 j = minTY; j < maxTY; j++)
    {
        for (uint32 i = minTX; i < maxTX; i++)
        {
            if (levelValueAtTile(level, i, j) == WALL)
            {
                wallRect.min = levelGridToWorld(level, i, j+1);
                wallRect.max = levelGridToWorld(level, i+1, j );
                if(rectCollidesRect(rect, wallRect))
                {
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
                       Vec2& collisionPoint, Vec2& normal )
{
    Vec2 nPos = position + velocity*dt;
    
    Vec2 halfSize = 0.5f*size;
    // Find the zone containing the rect
    uint32 minTX = min((uint32)(position.x - halfSize.x), (uint32)(nPos.x - halfSize.x)) ;
    uint32 maxTX = max((uint32)(position.x + halfSize.x), (uint32)(nPos.x + halfSize.x)) + 1;
    uint32 minTY = min((uint32)(level->height - position.y - halfSize.y), (uint32)(level->height - nPos.y - halfSize.y));
    uint32 maxTY = max((uint32)(level->height - position.y + halfSize.y), (uint32)(level->height - nPos.y + halfSize.y))+1;
    
    // Some values usefull for the computation of each rect
    real32 maxDist = length(velocity)*dt;
    Vec2 d = normalize(velocity);
    real32 fx = 1.0f / d.x;
    real32 fy = 1.0f / d.y;
    
    // Find the distance to the closest intersection
    // And the index of the box
    real32 tMin = 10000.0f;
    uint32 iC;
    uint32 jC;
    
    bool collides = false;
    for (uint32 j = minTY; j < maxTY; j++)
    {
        for (uint32 i = minTX; i < maxTX; i++)
        {
            if(levelValueAtTile(level, i, j) == WALL)
            {
                // B rect, unit rect of the level grown by the test rect size for minkowski
                Vec2 bMin = levelToWorld(level, (real32)i - halfSize.x, (real32)j + halfSize.y + 1.0f);
                Vec2 bMax = levelToWorld(level, (real32)i + halfSize.x + 1.0f, (real32)j - halfSize.y);
                
                real32 txm = (bMin.x-position.x) * fx;
                real32 txM = (bMax.x-position.x) * fx;
                real32 tym = (bMin.y-position.y) * fy;
                real32 tyM = (bMax.y-position.y) * fy;
                
                real32 tmin = max(min(txm,txM), min(tym,tyM));
                real32 tmax = min(max(txm,txM), max(tym,tyM));
                
                //ASSERT(!(tmin!=tmin));
                //ASSERT(!(tmax!=tmax));
                
                if(tmin < tmax && tmax > 0)
                {
                    if(tmin < maxDist && tmin < tMin)
                    {
                        collides = true;
                        tMin = tmin;
                        iC = i;
                        jC = j;
                    }
                }
            }
        }
    }
    
    if(collides)
    {
        ASSERT(!(tMin!=tMin));
        collisionPoint = position + tMin * d;
        
        // Compute the normal
        Vec2 bMin = levelToWorld(level, (real32)iC - halfSize.x, (real32)jC + halfSize.y + 1.0f);
        Vec2 bMax = levelToWorld(level, (real32)iC + halfSize.x + 1.0f, (real32)jC - halfSize.y);
        
        if(collisionPoint.x == bMin.x)
        {
            normal = Vec2(-1, 0);
        }
        else if(collisionPoint.x == bMax.x)
        {
            normal = Vec2(1, 0);
        }
        else if(collisionPoint.y == bMin.y)
        {
            normal = Vec2(0, -1);
        }
        else if(collisionPoint.y == bMax.y)
        {
            normal = Vec2(0, 1);
        }
        else
        {
            ASSERT(0);
        }
        
        return true;
    }
    
    return false;
}


// TODO: Free tiles memory
bool loadLevel(Level* level, const char* filename)
{
    SDL_Surface* levelBitmap = SDL_LoadBMP(filename);
    if (levelBitmap)
    {
        uint32 LEVEL_WIDTH = levelBitmap->w;
        uint32 LEVEL_HEIGHT = levelBitmap->h;
        uint8* tiles = new uint8[LEVEL_WIDTH*LEVEL_WIDTH];
        memset(tiles, 0, LEVEL_HEIGHT*LEVEL_WIDTH*sizeof(uint8));
        
        ASSERT(levelBitmap->format->BytesPerPixel == 4);
        ASSERT(levelBitmap->format->Amask == 0x000000ff);
        ASSERT(levelBitmap->format->Rmask == 0xff000000);
        ASSERT(levelBitmap->format->Gmask == 0x00ff0000);
        ASSERT(levelBitmap->format->Bmask == 0x0000ff00);
        uint32* pixels = (uint32*)levelBitmap->pixels;
        
        for (uint32 j = 0; j < LEVEL_HEIGHT; j++)
        {
            for (uint32 i = 0; i < LEVEL_WIDTH; i++)
            {
                uint32 index = i+j*LEVEL_WIDTH;
                uint32 pixel = pixels[index];
                if(pixel == 0x000000ff)     // Black
                {
                    tiles[index] = TILE_TYPE::WALL;
                }
                else if(pixel == 0xff0000ff) // Red
                {
                    tiles[index] = TILE_TYPE::MACHINE_GUN;
                }
                else if(pixel == 0x00ff00ff) // Green
                {
                    tiles[index] = TILE_TYPE::SHOTGUN;
                }
                else if(pixel == 0x0000ffff) // Blue
                {
                    tiles[index] = TILE_TYPE::BOMB;
                }
                else if(pixel == 0xffff00ff) // Yellow
                {
                    tiles[index] = TILE_TYPE::HEART;
                }
                else
                {
                    tiles[index] = TILE_TYPE::MAX_ENTITY_TYPE;
                }
            }
        }
        level->tiles = tiles;
        level->width = LEVEL_WIDTH;
        level->height = LEVEL_HEIGHT;
        
        return true;
    }
    else
    {
        printf("loadBitmap: %s\n", SDL_GetError());
        return false;
    }
}