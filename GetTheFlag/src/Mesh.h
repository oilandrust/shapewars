#ifndef OBJMESHH
#define OBJMESHH

#include "GetTheFlag.h"
#include "Vec3.h"
#include "Vec2.h"

struct Mesh3D {
    Vec3* positions;
    Vec3* normals;
    Vec3* colors;
    Vec2* uvs;
    uint32* indices;
    
    uint32 vCount;
    uint32 fCount;
};

void* loadObjMesh(Mesh3D* mesh, void* memoryPool, const char* filename, bool perVertexColors = false);

void* createCube(Mesh3D* mesh, void* memoryPool);

#endif