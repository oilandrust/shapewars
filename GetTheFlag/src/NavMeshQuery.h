//
//  NavMeshQuery.h
//  GetTheFlag
//
//  Created by Olivier on 24/12/15.
//
//

#ifndef NavMeshQuery_h
#define NavMeshQuery_h

#include "GetTheFlag.h"
#include "NavMesh.h"

uint32 findContainingPolygon(NavMesh* navMesh, const Vec3& point);

struct Path {
    uint32 length;
    Vec3* points;

    uint32 polyPathLength;
    uint32* polys;
};

bool findPath(NavMesh* navMesh, const Vec3& start, const Vec3& end, Path* path);

#endif /* NavMeshQuery_h */
