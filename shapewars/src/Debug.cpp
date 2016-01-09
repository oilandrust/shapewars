//
//  debug.cpp
//  GetTheFlag
//
//  Created by Olivier on 27/12/15.
//
//

#include "Mat3.h"
#include "Vec3.h"
#include "debug.h"

void debugHandleInput(Debug* debug, Input* input)
{
    if (input->keyStates[DEBUG_SHOW_DISTANCE_FIELD].clicked) {
        debug->showDistanceField = !debug->showDistanceField;
        if (debug->showDistanceField)
            debug->showRegions = false;
    }
    if (input->keyStates[DEBUG_SHOW_REGIONS].clicked) {
        debug->showRegions = !debug->showRegions;
        if (debug->showRegions)
            debug->showDistanceField = false;
    }
    if (input->keyStates[DEBUG_SHOW_CONTOURS].clicked) {
        debug->showContours = !debug->showContours;
        if (debug->showContours)
            debug->showTriRegions = false;
    }
    if (input->keyStates[DEBUG_SHOW_TRI_REGIONS].clicked) {
        debug->showTriRegions = !debug->showTriRegions;
        if (debug->showTriRegions)
            debug->showContours = false;
    }
    if (input->keyStates[DEBUG_SHOW_POLY_REGIONS].clicked) {
        debug->showNavMesh = !debug->showNavMesh;
        if (debug->showNavMesh)
            debug->showContours = false;
    }
    if (input->keyStates[DEBUG_SHOW_DUAL_MESH].clicked) {
        debug->showDual = !debug->showDual;
    }
    if (input->keyStates[DEBUG_SHOW_PATH].clicked) {
        debug->showPath = !debug->showPath;
    }
}

void debugDraw(Debug* debug, Renderer* renderer)
{
    Mat3 identity3;
    identity(identity3);
    Vec3 groundSize(debug->planeSize);
    Vec3 groundCenter(.5f * debug->planeSize, .5f * debug->planeSize, 0.f);

    if (debug->showDistanceField) {
        pushMeshPieceTextured(renderer, &renderer->texDiffShader, debug->planeVao, debug->planeICount,
            debug->distanceFieldTexId, identity3, groundSize, groundCenter);
    }
    else if (debug->showRegions) {
        pushMeshPieceTextured(renderer, &renderer->texDiffShader, debug->planeVao, debug->planeICount,
            debug->idsTexId, identity3, groundSize, groundCenter);
    }

    if (debug->showContours) {
        for (uint32 i = 0; i < debug->contourCount; i++) {
            pushArrayPiece(renderer, &renderer->flatColorShader,
                debug->contourVaos[i], debug->contourICounts[i], ARRAY_POINTS,
                identity3, Vec3(1.f), Vec3(0.f), Vec3(1.f));
        }
        for (uint32 i = 0; i < debug->contourCount; i++) {
            pushArrayPiece(renderer, &renderer->flatColorShader,
                debug->contourVaos[i], debug->contourICounts[i], ARRAY_LINE_LOOP,
                identity3, Vec3(1.f), Vec3(0.f), Vec3(1.f));
        }
    }

    if (debug->showTriRegions) {
        for (uint32 i = 0; i < debug->contourMeshCount; i++) {
            pushMeshPieceWireframe(renderer, &renderer->flatColorShader,
                debug->contourMeshes[i], debug->contourMeshesIndices[i],
                identity3, Vec3(1.0f), Vec3(0.f), Vec3(1.0f));
        }
    }

    if (debug->showNavMesh) {
        for (uint32 i = 0; i < debug->navPolyCount; i++) {
            pushIndexedArrayPiece(renderer, &renderer->flatColorShader, debug->polyVaos[i], debug->polyICounts[i], INDEXED_ARRAY_LINE_LOOP,
                identity3, Vec3(1.0f), Vec3(.0f), Vec3(0, 0, 1));
        }
    }

    if (debug->showDual) {
        pushIndexedArrayPiece(renderer, &renderer->flatColorShader, debug->dualVao, debug->dualICount, INDEXED_ARRAY_LINES,
            identity3, Vec3(1.0f), Vec3(.0f), Vec3(1, 0, 0));
    }

    if (debug->showNavMesh && debug->showPath) {
        for (uint32 p = 0; p < debug->path->polyPathLength; p++) {
            uint32 iCount = polyVertCount(debug->navMesh, debug->path->polys[p]);
            pushIndexedArrayPiece(renderer, &renderer->flatColorShader, debug->polyVaos[debug->path->polys[p]], iCount, INDEXED_ARRAY_LINE_LOOP,
                identity3, Vec3(1.0f), Vec3(.0f), Vec3(1, 0, 0));
        }
    }

    if (debug->showPath) {
        pushArrayPiece(renderer, &renderer->flatColorShader,
            debug->pathVao, debug->pathLength, ARRAY_LINE_STRIP,
            identity3, Vec3(1.f), Vec3(0.f), Vec3(1.f));
    }
}