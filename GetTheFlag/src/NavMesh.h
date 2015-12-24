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
    
    uint32 vertCount;
    uint32 polyCount;
    uint32 maxVertPerPoly;
};

#endif /* NavMesh_h */
