
#ifndef NavMeshGen_h
#define NavMeshGen_h

#include "GetTheFlag.h"
#include "Level.h"
#include "OpenGLClient.h"

struct DistanceField {
    uint32 width;
    uint32 height;
    
    real32* field;
    real32 minVal;
    
    // For visualizion
    Texture texture;
};

void genDistanceField(MemoryArena* arena, Level* level, DistanceField* field);

struct RegionIdMap {
    uint32 width;
    uint32 height;
    int32* ids;
    uint32 regionCount;
    int32 lastId;
    
    Texture texture;
};

struct Contour {
    Vec3* vertices;
    uint32 count;
};

struct Contours {
    Contour* contours;
    uint32 count;
};

void genRegions(MemoryArena* arena, DistanceField* distanceFiend, RegionIdMap* regions);

void genContours(MemoryArena* arena, RegionIdMap* regions, Contours* contour);


#endif /* NavMeshGen_h */
