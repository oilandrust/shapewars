#include "Level.h"

#include <SDL_surface.h>
#include <SDL_error.h>

bool levelRectCollides(Level* level, const Rect& rect)
{
    // Find the zone containing the rect
    uint32 minTX = (uint32)rect.min.x;
    uint32 maxTX = (uint32)rect.max.x+1;
    uint32 mintTY = (uint32)rect.min.y;
    uint32 maxTY = (uint32)rect.max.y+1;
    
    Rect wallRect = {
        Vec2(0,0),
        Vec2(1,1)
    };
    for (uint32 j = mintTY; j < maxTY; j++)
    {
        for (uint32 i = minTX; i < maxTX; i++)
        {
            if (levelValueAtTile(level, i, j) == WALL)
            {
                wallRect.min = Vec2(i, j);
                wallRect.max = Vec2(i+1,j+1);
                if(rectCollidesRect(rect, wallRect))
                {
                    return true;
                }
            }
        }
    }
    
    return false;
}


bool levelRectCollides(Level* level, const Rect& a, Vec2& penetration)
{
    // Find the zone containing the rect
    uint32 minTX = (uint32)a.min.x;
    uint32 maxTX = (uint32)a.max.x+1;
    uint32 mintTY = (uint32)a.min.y;
    uint32 maxTY = (uint32)a.max.y+1;
    
    Rect b = {
        Vec2(0,0),
        Vec2(1,1)
    };
    
    bool collides = false;
    for (uint32 j = mintTY; j < maxTY; j++)
    {
        for (uint32 i = minTX; i < maxTX; i++)
        {
            if (levelValueAtTile(level, i, j) == WALL)
            {
                b.min = Vec2(i,j);
                b.max = Vec2(i+1,j+1);
                
                if(rectCollidesRect(a, b))
                {
                    collides = true;
                    
                    if(a.min.x < b.max.x && a.max.x > b.max.x)
                    {
                        // a is inside b from the right
                        penetration.x = a.min.x - b.max.x;
                    }
                    else if(a.max.x > b.min.x && a.min.x < b.min.x)
                    {
                        // a is inside b from the left
                        penetration.x = a.max.x - b.min.x;
                    }
                    
                    if(a.min.y < b.max.y && a.max.y > b.max.y)
                    {
                        // a is inside b from the top
                        penetration.y = a.min.y - b.max.y;
                    }
                    else if(a.max.y > b.min.y && a.min.y < b.min.y)
                    {
                        // a is inside b from the bottom
                        penetration.y = a.max.y - b.min.y;
                    }
                    
                    if (abs(penetration.y) > abs(penetration.x))
                    {
                        penetration.y = 0;
                    }
                    else
                    {
                        penetration.x = 0;
                    }
                }
            }
        }
    }
    
    return collides;
}


// TODO: Free tiles memory
bool loadLevel(Level* level)
{
    SDL_Surface* levelBitmap = SDL_LoadBMP("data/testlvl.bmp");
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