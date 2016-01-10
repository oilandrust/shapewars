#include "Mesh.h"
#include <cstdio>

void createCube(MemoryArena* arena, Mesh3D* mesh)
{
    uint32 vCount = 24;
    uint32 fCount = 12;

    mesh->vCount = vCount;
    mesh->fCount = fCount;

    Vec3* p = pushArray<Vec3>(arena, vCount);
    Vec3* n = pushArray<Vec3>(arena, vCount);
    uint32* i = pushArray<uint32>(arena, 3 * fCount);

    mesh->positions = p;
    mesh->normals = n;
    mesh->indices = i;

    // TODO: Check the orientation of all faces with backface culling

    uint32 f = 0;
    // +Z
    *p++ = Vec3(-.5f, -.5f, .5f);
    *n++ = Vec3(0, 0, 1);
    *p++ = Vec3(.5f, -.5f, .5f);
    *n++ = Vec3(0, 0, 1);
    *p++ = Vec3(.5f, .5f, .5f);
    *n++ = Vec3(0, 0, 1);
    *p++ = Vec3(-.5f, .5f, .5f);
    *n++ = Vec3(0, 0, 1);
    *i++ = f;
    *i++ = f + 1;
    *i++ = f + 2;
    *i++ = f;
    *i++ = f + 2;
    *i++ = f + 3;
    f += 4;

    // -Z
    *p++ = Vec3(-.5f, -.5f, -.5f);
    *n++ = Vec3(0, 0, -1);
    *p++ = Vec3(-.5f, .5f, -.5f);
    *n++ = Vec3(0, 0, -1);
    *p++ = Vec3(.5f, .5f, -.5f);
    *n++ = Vec3(0, 0, -1);
    *p++ = Vec3(.5f, -.5f, -.5f);
    *n++ = Vec3(0, 0, -1);
    *i++ = f;
    *i++ = f + 1;
    *i++ = f + 2;
    *i++ = f;
    *i++ = f + 2;
    *i++ = f + 3;
    f += 4;

    // +X
    *p++ = Vec3(.5f, -.5f, -.5f);
    *n++ = Vec3(1, 0, 0);
    *p++ = Vec3(.5f, .5f, -.5f);
    *n++ = Vec3(1, 0, 0);
    *p++ = Vec3(.5f, .5f, .5f);
    *n++ = Vec3(1, 0, 0);
    *p++ = Vec3(.5f, -.5f, .5f);
    *n++ = Vec3(1, 0, 0);
    *i++ = f;
    *i++ = f + 1;
    *i++ = f + 2;
    *i++ = f;
    *i++ = f + 2;
    *i++ = f + 3;
    f += 4;

    // -X
    *p++ = Vec3(-.5f, -.5f, -.5f);
    *n++ = Vec3(-1, 0, 0);
    *p++ = Vec3(-.5f, -.5f, .5f);
    *n++ = Vec3(-1, 0, 0);
    *p++ = Vec3(-.5f, .5f, .5f);
    *n++ = Vec3(-1, 0, 0);
    *p++ = Vec3(-.5f, .5f, -.5f);
    *n++ = Vec3(-1, 0, 0);
    *i++ = f;
    *i++ = f + 1;
    *i++ = f + 2;
    *i++ = f;
    *i++ = f + 2;
    *i++ = f + 3;
    f += 4;

    // -Y
    *p++ = Vec3(-.5f, -.5f, -.5f);
    *n++ = Vec3(0, -1, 0);
    *p++ = Vec3(.5f, -.5f, -.5f);
    *n++ = Vec3(0, -1, 0);
    *p++ = Vec3(.5f, -.5f, .5f);
    *n++ = Vec3(0, -1, 0);
    *p++ = Vec3(-.5f, -.5f, .5f);
    *n++ = Vec3(0, -1, 0);
    *i++ = f;
    *i++ = f + 1;
    *i++ = f + 2;
    *i++ = f;
    *i++ = f + 2;
    *i++ = f + 3;
    f += 4;

    // +Y
    *p++ = Vec3(-.5f, .5f, -.5f);
    *n++ = Vec3(0, 1, 0);
    *p++ = Vec3(-.5f, .5f, .5f);
    *n++ = Vec3(0, 1, 0);
    *p++ = Vec3(.5f, .5f, .5f);
    *n++ = Vec3(0, 1, 0);
    *p++ = Vec3(.5f, .5f, -.5f);
    *n++ = Vec3(0, 1, 0);
    *i++ = f;
    *i++ = f + 1;
    *i++ = f + 2;
    *i++ = f;
    *i++ = f + 2;
    *i++ = f + 3;
    f += 4;
}

void pushBox(Vec3* verts, Vec3* norms, uint32* inds, const Vec3& center, const Vec3& size, uint32& vCount, uint32& iCount)
{
    Vec3* p = verts + vCount;
    Vec3* n = norms + vCount;
    uint32* i = inds + iCount;

    uint32 f = vCount;
    // +Z
    *p++ = size * Vec3(-.5f, -.5f, .5f) + center;
    *n++ = Vec3(0, 0, 1);
    *p++ = size * Vec3(.5f, -.5f, .5f) + center;
    *n++ = Vec3(0, 0, 1);
    *p++ = size * Vec3(.5f, .5f, .5f) + center;
    *n++ = Vec3(0, 0, 1);
    *p++ = size * Vec3(-.5f, .5f, .5f) + center;
    *n++ = Vec3(0, 0, 1);
    *i++ = f;
    *i++ = f + 1;
    *i++ = f + 2;
    *i++ = f;
    *i++ = f + 2;
    *i++ = f + 3;
    f += 4;

    // -Z
    *p++ = size * Vec3(-.5f, -.5f, -.5f) + center;
    *n++ = Vec3(0, 0, -1);
    *p++ = size * Vec3(-.5f, .5f, -.5f) + center;
    *n++ = Vec3(0, 0, -1);
    *p++ = size * Vec3(.5f, .5f, -.5f) + center;
    *n++ = Vec3(0, 0, -1);
    *p++ = size * Vec3(.5f, -.5f, -.5f) + center;
    *n++ = Vec3(0, 0, -1);
    *i++ = f;
    *i++ = f + 1;
    *i++ = f + 2;
    *i++ = f;
    *i++ = f + 2;
    *i++ = f + 3;
    f += 4;

    // +X
    *p++ = size * Vec3(.5f, -.5f, -.5f) + center;
    *n++ = Vec3(1, 0, 0);
    *p++ = size * Vec3(.5f, .5f, -.5f) + center;
    *n++ = Vec3(1, 0, 0);
    *p++ = size * Vec3(.5f, .5f, .5f) + center;
    *n++ = Vec3(1, 0, 0);
    *p++ = size * Vec3(.5f, -.5f, .5f) + center;
    *n++ = Vec3(1, 0, 0);
    *i++ = f;
    *i++ = f + 1;
    *i++ = f + 2;
    *i++ = f;
    *i++ = f + 2;
    *i++ = f + 3;
    f += 4;

    // -X
    *p++ = size * Vec3(-.5f, -.5f, -.5f) + center;
    *n++ = Vec3(-1, 0, 0);
    *p++ = size * Vec3(-.5f, -.5f, .5f) + center;
    *n++ = Vec3(-1, 0, 0);
    *p++ = size * Vec3(-.5f, .5f, .5f) + center;
    *n++ = Vec3(-1, 0, 0);
    *p++ = size * Vec3(-.5f, .5f, -.5f) + center;
    *n++ = Vec3(-1, 0, 0);
    *i++ = f;
    *i++ = f + 1;
    *i++ = f + 2;
    *i++ = f;
    *i++ = f + 2;
    *i++ = f + 3;
    f += 4;

    // -Y
    *p++ = size * Vec3(-.5f, -.5f, -.5f) + center;
    *n++ = Vec3(0, -1, 0);
    *p++ = size * Vec3(.5f, -.5f, -.5f) + center;
    *n++ = Vec3(0, -1, 0);
    *p++ = size * Vec3(.5f, -.5f, .5f) + center;
    *n++ = Vec3(0, -1, 0);
    *p++ = size * Vec3(-.5f, -.5f, .5f) + center;
    *n++ = Vec3(0, -1, 0);
    *i++ = f;
    *i++ = f + 1;
    *i++ = f + 2;
    *i++ = f;
    *i++ = f + 2;
    *i++ = f + 3;
    f += 4;

    // +Y
    *p++ = size * Vec3(-.5f, .5f, -.5f) + center;
    *n++ = Vec3(0, 1, 0);
    *p++ = size * Vec3(-.5f, .5f, .5f) + center;
    *n++ = Vec3(0, 1, 0);
    *p++ = size * Vec3(.5f, .5f, .5f) + center;
    *n++ = Vec3(0, 1, 0);
    *p++ = size * Vec3(.5f, .5f, -.5f) + center;
    *n++ = Vec3(0, 1, 0);
    *i++ = f;
    *i++ = f + 1;
    *i++ = f + 2;
    *i++ = f;
    *i++ = f + 2;
    *i++ = f + 3;
    f += 4;

    vCount += 4 * 6;
    iCount += 36;
}

void createPlane(MemoryArena* arena, Mesh3D* mesh)
{
    uint32 vCount = 4;
    uint32 fCount = 2;

    mesh->vCount = vCount;
    mesh->fCount = fCount;

    Vec3* p = pushArray<Vec3>(arena, vCount);
    Vec3* n = pushArray<Vec3>(arena, vCount);
    Vec2* u = pushArray<Vec2>(arena, vCount);
    uint32* i = pushArray<uint32>(arena, 3 * fCount);

    mesh->positions = p;
    mesh->normals = n;
    mesh->uvs = u;
    mesh->indices = i;

    uint32 f = 0;
    *p++ = Vec3(-.5f, -.5f, .0f);
    *n++ = Vec3(0, 0, 1);
    *u++ = Vec2(0, 0);

    *p++ = Vec3(.5f, -.5f, .0f);
    *n++ = Vec3(0, 0, 1);
    *u++ = Vec2(1, 0);

    *p++ = Vec3(.5f, .5f, .0f);
    *n++ = Vec3(0, 0, 1);
    *u++ = Vec2(1, 1);

    *p++ = Vec3(-.5f, .5f, .0f);
    *n++ = Vec3(0, 0, 1);
    *u++ = Vec2(0, 1);

    *i++ = f;
    *i++ = f + 1;
    *i++ = f + 2;
    *i++ = f;
    *i++ = f + 2;
    *i++ = f + 3;
}

void computeSmoothVertexNormals(Vec3* n, Vec3* p, uint32* ind, uint32 vCount, uint32 fCount)
{
    // At this point we have
    // Positions/Indices/UVs

    // TODO: COMPUTE THE NORMALS
    // we need face normals temporarily so we do:
    // Position/Indices/UVs/Normals/FaceNormals
    // and we return a pointer to the end of the normals

    memset(n, 0, vCount * sizeof(Vec3));

    Vec3* fn = n + vCount;

    for (uint32 f = 0; f < fCount; f++) {
        uint32* face = ind + 3 * f;

        Vec3 a = p[face[1]] - p[face[0]];
        Vec3 b = p[face[2]] - p[face[0]];

        fn[f] = normalize(cross(a, b));

        n[face[0]] += fn[f];
        n[face[1]] += fn[f];
        n[face[2]] += fn[f];
    }

    for (uint32 v = 0; v < vCount; v++) {
        n[v] = normalize(n[v]);
    }
}
