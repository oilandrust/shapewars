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

#endif /* NavMesh_h */
