
#ifndef NavMeshGen_h
#define NavMeshGen_h

#include "GetTheFlag.h"
#include "NavMesh.h"
#include "OpenGLClient.h"

struct LevelRaster {
    uint8* raster;
    uint32 width;
    uint32 height;
};

struct DistanceField {
    uint32 width;
    uint32 height;

    real32* field;
    real32 minVal;

    // FIXME: Remove texture from here
    // For visualizion
    Texture texture;
};

void genDistanceField(MemoryArena* arena, LevelRaster* level, DistanceField* field);

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

struct DualMesh {
    Vec3* vertices;
    uint32 vertCount;
    uint32* indices;
    uint32 indCount;
};

void buildNavMesh(MemoryArena* arena, ContourSet* contours, Mesh3D* triMeshes,
    NavMesh* mesh, DualMesh* dual);

bool checkNavMesh(NavMesh* mesh);

uint32 polyVertCount(NavMesh* mesh, uint32 polyRef);

#endif /* NavMeshGen_h */
