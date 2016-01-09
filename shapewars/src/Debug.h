//
//  Debug.h
//  GetTheFlag
//
//  Created by Olivier on 27/12/15.
//
//

#ifndef Debug_h
#define Debug_h

#include "Input.h"
#include "NavMeshQuery.h"
#include "Renderer.h"

struct Debug {
    bool showDistanceField = false;
    bool showRegions = false;
    GLuint planeVao;
    GLuint planeICount;
    real32 planeSize;
    GLuint distanceFieldTexId;
    GLuint idsTexId;

    bool showContours = false;
    uint32 contourCount;
    GLuint* contourVaos;
    uint32* contourICounts;

    bool showTriRegions = false;
    uint32 contourMeshCount;
    GLuint* contourMeshes;
    uint32* contourMeshesIndices;

    bool showNavMesh = false;
    uint32 navPolyCount;
    GLuint* polyVaos;
    uint32* polyICounts;

    bool showDual = false;
    GLuint dualVao;
    uint32 dualICount;

    Path* path;
    NavMesh* navMesh;

    bool showPath = false;
    GLuint pathVao;
    GLuint pathVbo;
    uint32 pathLength;
};

void debugHandleInput(Debug* debug, Input* input);

void debugDraw(Debug* debug, Renderer* renderer);

#endif /* Debug_h */
