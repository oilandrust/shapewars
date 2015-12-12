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

inline uint16 read_uint16(void* buffer)
{
    return *(uint16*)buffer;
}

inline uint32 read_uint32(void* buffer)
{
    return *(uint32*)buffer;
}

inline real32 read_real32(void* buffer)
{
    return *(real32*)buffer;
}

inline uint16 read3DSChunkId(void* buffer)
{
    return read_uint16(buffer);
}

inline uint32 read3DSChunkLength(void* buffer)
{
    return read_uint32((char*)buffer + 2);
}

inline uint8* nextChunk(uint8* chunk, uint32 length)
{
    return chunk + length;
}

inline uint8* subChunk(uint8* chunk)
{
    return chunk + 6;
}

void* loadObjMesh(Mesh3D* mesh, void* memoryPool, const char* filename, bool perVertexColors = false);

void* load3DSMesh(Mesh3D* mesh, void* memoryPool, const char* filename);

void* loadFBXMesh(Mesh3D* mesh, void* memoryPool, const char* filename);

void createCube(MemoryArena* arena, Mesh3D* mesh);

void createPlane(MemoryArena* arena, Mesh3D* mesh);


#endif