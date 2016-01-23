
#ifndef NavMeshGen_h
#define NavMeshGen_h

#include "Debug.h"
#include "Level.h"
#include "NavMesh.h"
#include "ShapeWars.h"

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
};

void initializeNavMesh(Memory* memory, Debug* debug, Level* level, NavMesh* navMesh, uint32 fieldWidth, uint32 fieldHeight, real32 radius);

void genDistanceField(MemoryArena* arena, LevelRaster* level, DistanceField* field);

GLuint debugDistanceFieldCreateTexture(MemoryArena* arena, DistanceField* field);

struct RegionIdMap {
    uint32 width;
    uint32 height;
	real32 cellWidth;
	real32 cellHeight;
    int32* ids;
    uint32 regionCount;
    int32 lastId;
};

void genRegions(MemoryArena* arena, DistanceField* distanceFiend, RegionIdMap* regions, real32 radius);

GLuint debugRegionsCreateTexture(MemoryArena* arena, RegionIdMap* idMap);

struct Contour {
    Vec3* vertices;
	bool* isVertex;
    uint32 count;
};

struct ContourSet {
    Contour* contours;
    uint32 count;
};

void walkCountour(MemoryArena* arena, Contour* contour, int32* ids, uint32 w, uint32 h, real32 mw, real32 mh, real32 cw, real32 ch, uint32 i, uint32 j);

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

void buildNavMesh(Memory* memory, ContourSet* contours, Mesh3D* triMeshes,
    NavMesh* mesh, DualMesh* dual);

bool checkNavMesh(NavMesh* mesh);

uint32 polyVertCount(NavMesh* mesh, uint32 polyRef);

#endif /* NavMeshGen_h */
