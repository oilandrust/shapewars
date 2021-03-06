#ifndef OBJMESHH
#define OBJMESHH

#include "ShapeWars.h"
#include "Vec2.h"
#include "Vec3.h"

struct Mesh3D {
    Vec3* positions;
    Vec3* normals;
    Vec3* colors;
    Vec2* uvs;

    uint32* indices;

    uint32 vCount;
    uint32 fCount;
};

void createCube(MemoryArena* arena, Mesh3D* mesh);

void pushBox(Vec3* verts, Vec3* norms, uint32* inds, const Vec3& center, const Vec3& size, uint32& vCount, uint32& iCount);

void createPlane(MemoryArena* arena, Mesh3D* mesh);

#endif