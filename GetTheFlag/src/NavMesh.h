//
//  NavMesh.h
//  GetTheFlag
//
//  Created by Olivier on 24/12/15.
//
//

#ifndef NavMesh_h
#define NavMesh_h

#include "GetTheFlag.h"
#include "Vec3.h"

#define NULL_INDEX 0xffffffff

struct NavMesh {
    Vec3* vertices;
    uint32* polygons; // 2*maxVertPerPoly*polyCount
    Vec3* polyCenters;

    uint32 vertCount;
    uint32 polyCount;
    uint32 maxVertPerPoly;
};

inline uint32 next(uint32 i, uint32 max)
{
    return (i + 1) % max;
}

inline uint32 prev(uint32 i, uint32 max)
{
    return i == 0 ? max - 1 : i - 1;
}

inline bool isCCW(const Vec3& p0, const Vec3& p1, const Vec3& p2)
{
    return cross(p1 - p0, p2 - p1).z > 0;
}

inline bool isCW(const Vec3& p0, const Vec3& p1, const Vec3& p2)
{
    return cross(p1 - p0, p2 - p1).z < 0;
}

inline uint32 polyVertCount(uint32 mvp, uint32* poly)
{
    uint32 pvCount = 0;
    while (poly[pvCount] != NULL_INDEX && pvCount < mvp) {
        pvCount++;
    }
    return pvCount;
}

inline uint32 polyVertCount(NavMesh* mesh, uint32 polyRef)
{
    ASSERT(polyRef < mesh->polyCount);
    uint32* poly = mesh->polygons + 2 * polyRef * mesh->maxVertPerPoly;
    return polyVertCount(mesh->maxVertPerPoly, poly);
}

#endif /* NavMesh_h */
