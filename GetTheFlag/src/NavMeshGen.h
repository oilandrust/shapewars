
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
    
    // FIXME: Remove texture from here
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

    // FIXME: Remove texture from here
    Texture texture;
};

void genRegions(MemoryArena* arena, DistanceField* distanceFiend, RegionIdMap* regions);

struct Contour {
    Vec3* vertices;
    uint32 count;
};

struct ContourSet {
    Contour* contours;
    uint32 count;
};
// FIXME: Keep ids in contours for rendering
void genContours(MemoryArena* arena, RegionIdMap* regions, ContourSet* contour);

// FIXME: Remove Mesh3D from here
void triangulateContours(MemoryArena* arena, ContourSet* contours, Mesh3D* meshes);

#define NULL_INDEX 0xffffffff

struct NavMesh {
    Vec3* vertices;
    uint32* polygons; // 2*maxVertPerPoly*polyCount
    
    uint32 vertCount;
    uint32 polyCount;
    uint32 maxVertPerPoly;
};

void buildNavMesh(MemoryArena* arena, ContourSet* contours, Mesh3D* triMeshes, NavMesh* mesh);


#endif /* NavMeshGen_h */
