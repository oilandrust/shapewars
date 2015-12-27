//
//  NavMeshQuery.cpp
//  GetTheFlag
//
//  Created by Olivier on 24/12/15.
//
//

#include "NavMeshQuery.h"

struct Node {
    real32 cost;
    real32 estimate;
    uint32 pos;
    uint32 step;
};

struct Heap {
    Node nodes[1024];
    uint32 next = 0;
};

static inline uint32 parent(uint32 node)
{
    return (node - 1) / 2;
}

static inline uint32 left(uint32 node)
{
    return 2 * node + 1;
}

static inline uint32 right(uint32 node)
{
    return 2 * node + 2;
}

static inline real32 cost(const Node& node)
{
    return node.cost + node.estimate;
}

static void push(Heap* heap, Node node)
{
    Node* nodes = heap->nodes;

    uint32 i = heap->next++;
    nodes[i] = node;

    while (i > 0) {
        uint32 p = parent(i);
        if (cost(nodes[i]) < cost(nodes[p])) {
            Node tmp = nodes[i];
            nodes[i] = nodes[p];
            nodes[p] = tmp;
            i = p;
        }
        else {
            return;
        }
    }
}

static Node top(Heap* heap)
{
    return heap->nodes[0];
}

static bool isEmpty(Heap* heap)
{
    return heap->next == 0;
}

static void pop(Heap* heap)
{
    Node* nodes = heap->nodes;
    nodes[0] = nodes[--heap->next];

    uint32 i = 0;
    uint32 length = heap->next;

    while (i < heap->next) {
        uint32 m = length;

        uint32 l = left(i);
        if (l < length) {
            m = l;
            uint32 r = right(i);
            if (r < length && cost(nodes[r]) < cost(nodes[l])) {
                m = r;
            }
        }

        if (m < length && cost(nodes[m]) < cost(nodes[i])) {
            Node tmp = nodes[i];
            nodes[i] = nodes[m];
            nodes[m] = tmp;
            i = m;
        }
        else {
            return;
        }
    }
}

static uint32 nextVertex(uint32* poly, uint32 mvp, uint32 i)
{
    uint32 ni = i + 1;
    return ni == mvp || poly[ni] == NULL_INDEX ? poly[0] : poly[ni];
}

bool pointInsidePoly(NavMesh* mesh, uint32 p, const Vec3& point)
{
    uint32 mvp = mesh->maxVertPerPoly;
    uint32* poly = mesh->polygons + 2 * mvp * p;
    Vec3* verts = mesh->vertices;

    for (uint32 i = 0; i < mvp && poly[i] != NULL_INDEX; i++) {
        uint32 i0 = poly[i];
        uint32 i1 = nextVertex(poly, mvp, i);

        if (!isCCW(verts[i0], verts[i1], point)) {
            return false;
        }
    }

    return true;
}

uint32 findContainingPolygon(NavMesh* navMesh, const Vec3& point)
{
    uint32 polyCount = navMesh->polyCount;

    for (uint32 p = 0; p < polyCount; p++) {
        if (pointInsidePoly(navMesh, p, point)) {
            return p;
        }
    }

    return NULL_INDEX;
}

#define UNVISITED 0xfffffffe
#define NO_PRED 0xffffffff

bool findPath(MemoryArena* arena, NavMesh* navMesh, const Vec3& start, const Vec3& end, Path* path)
{
    uint32 startPoly = findContainingPolygon(navMesh, start);
    if (startPoly == NULL_INDEX) {
        return false;
    }

    uint32 endPoly = findContainingPolygon(navMesh, end);
    if (endPoly == NULL_INDEX) {
        return false;
    }

    uint32 mvp = navMesh->maxVertPerPoly;
    uint32* polys = navMesh->polygons;

    Vec3* polyCenters = navMesh->polyCenters;

    Heap heap;
    real32 distStartEnd = length(polyCenters[startPoly] - polyCenters[endPoly]);
    push(&heap, { .0f, distStartEnd, startPoly, 0 });

    uint32* pred = pushArray<uint32>(arena, navMesh->polyCount);
    for (uint32 i = 0; i < navMesh->polyCount; i++) {
        pred[i] = UNVISITED;
    }

    pred[startPoly] = NO_PRED;
    bool found = false;
    uint32 pathLength = 0;

    while (!isEmpty(&heap)) {
        Node curr = top(&heap);

        if (curr.pos == endPoly) {
            found = true;
            pathLength = curr.step + 1;
            break;
        }

        uint32* currPoly = polys + 2 * mvp * curr.pos + mvp;
        pop(&heap);

        // for each neighboor
        for (uint32 n = 0; n < mvp; n++) {
            uint32 neighboor = currPoly[n];
            if (neighboor == NULL_INDEX) {
                continue;
            }

            if (pred[neighboor] == UNVISITED) {
                pred[neighboor] = curr.pos;
                real32 distNEnd = length(polyCenters[endPoly] - polyCenters[neighboor]);
                real32 stepDist = length(polyCenters[curr.pos] - polyCenters[neighboor]);
                push(&heap, { curr.cost + stepDist, distNEnd, neighboor, curr.step + 1 });
            }
        }
    }

    path->length = 0;
    if (found) {
        uint32* polyPath = path->polys;
        path->polyPathLength = pathLength;

        uint32 i = pathLength - 1;
        uint32 pos = endPoly;
        do {
            polyPath[i--] = pos;
            pos = pred[pos];
        } while (pos != NULL_INDEX);

        popArray<uint32>(arena, navMesh->polyCount);
        return true;
    }

    popArray<uint32>(arena, navMesh->polyCount);
    return false;
}

void pullString(MemoryArena* arena, NavMesh* mesh, const Vec3& start, const Vec3& end, Path* path)
{
    uint32* polys = path->polys;
    Vec3* pts = path->points;
    uint32 count = 0;

    pts[count++] = start;

    uint32 polyPathCount = path->polyPathLength;
    if (polyPathCount == 1) {
        pts[count++] = end;
        path->length = count;
        return;
    }

    uint32 portalCount = polyPathCount;
    Vec3* portals = pushArray<Vec3>(arena, 2 * portalCount);

    uint32* mPolys = mesh->polygons;
    uint32 mvp = mesh->maxVertPerPoly;

    // Collect the portals at each poly boundaries.
    for (uint32 p = 0; p < path->polyPathLength - 1; p++) {
        uint32 p0 = polys[p];
        uint32 p1 = polys[p + 1];

        uint32 i0 = NULL_INDEX;
        uint32 i1 = NULL_INDEX;

        uint32* poly0 = mPolys + 2 * p0 * mvp;
        for (uint32 i = 0; i < mvp && poly0[i] != NULL_INDEX; i++) {
            if (poly0[i + mvp] == p1) {
                i0 = poly0[i];
                i1 = nextVertex(poly0, mvp, i);
                break;
            }
        }

        ASSERT(i0 != NULL_INDEX);
        ASSERT(i1 != NULL_INDEX);

        portals[2 * p] = mesh->vertices[i0];
        portals[2 * p + 1] = mesh->vertices[i1];
    }
    portals[2 * (portalCount - 1)] = end;
    portals[2 * (portalCount - 1) + 1] = end;

    const Vec3* apex = &start;
    Vec3* port0 = &portals[0];
    Vec3* port1 = &portals[1];
    uint32 port0i = 0;
    uint32 port1i = 0;

    for (uint32 p = 1; p < portalCount; p++) {
        Vec3* p0 = portals + 2 * p;
        Vec3* p1 = portals + 2 * p + 1;

        if (isCCW(*apex, *port0, *p0)) {
            if (isCW(*apex, *port1, *p0)) {
                // Narrow the funnel.
                port0 = p0;
                port0i = p;
            }
            else {
                pts[count++] = *port1;

                // Restart the search if we haven't reached destination.
                if (port1i == portalCount - 1) {
                    break;
                }
                apex = port1;
                port0i = port1i + 1;
                port0 = portals + 2 * port0i;
                port1i = port1i + 1;
                port1 = portals + 2 * port1i + 1;
                p = port1i;
                continue;
            }
        }

        if (isCW(*apex, *port1, *p1)) {
            if (isCCW(*apex, *port0, *p1)) {
                port1 = p1;
                port1i = p;
            }
            else {
                pts[count++] = *port0;

                // Restart the search if we haven't reached destination.
                if (port0i == portalCount - 1) {
                    break;
                }
                apex = port0;
                port1i = port0i + 1;
                port1 = portals + 2 * port1i + 1;
                port0i = port0i + 1;
                port0 = portals + 2 * port0i;
                p = port0i;
                continue;
            }
        }
    }

    pts[count++] = end;
    path->length = count;

    popArray<Vec3>(arena, 2 * portalCount);
}
