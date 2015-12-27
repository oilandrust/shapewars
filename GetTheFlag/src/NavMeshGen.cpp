//
//  NavMeshGen.cpp
//  GetTheFlag
//
//  Created by Olivier on 05/12/15.
//
//

#include "NavMeshGen.h"

void genDistanceField(MemoryArena* arena, LevelRaster* level, DistanceField* field)
{
    field->width = level->width;
    field->height = level->height;

    uint8* levelMap = level->raster;

    uint32 width = field->width;
    uint32 height = field->height;
    size_t count = width * height;

    real32* distanceField = pushArray<real32>(arena, count);
    uint8* distanceFieldTexData = pushArray<uint8>(arena, count);
    field->field = distanceField;

    // Retrieve the position of the walls.
    uint32 wallCount = 0;
    for (uint32 j = 0; j < count; j++) {
        if (levelMap[j])
            wallCount++;
    }
    Vec2* walls = pushArray<Vec2>(arena, wallCount);
    uint32 w = 0;
    for (uint32 j = 0; j < height; j++) {
        for (uint32 i = 0; i < width; i++) {
            if (levelMap[i + j * width])
                walls[w++] = Vec2(i, j);
        }
    }

    // Compute the distance field.
    real32 maxDist = 0;
    for (uint32 j = 0; j < height; j++) {
        for (uint32 i = 0; i < width; i++) {
            Vec2 pt(i, j);
            real32 minDist = 10000000;
            for (uint32 w = 0; w < wallCount; w++) {
                real32 dist = length(pt - walls[w]);
                if (dist < minDist) {
                    minDist = dist;
                }
            }
            distanceField[i + j * width] = -minDist;
            if (minDist > maxDist) {
                maxDist = minDist;
            }
        }
    }
    field->minVal = -maxDist;

    popArray<Vec2>(arena, wallCount);

    // Update the texture data.
    for (uint32 j = 0; j < height; j++) {
        for (uint32 i = 0; i < width; i++) {
            distanceFieldTexData[i + j * width] = -255 * distanceField[i + j * width] / maxDist;
        }
    }
    field->texture.width = width;
    field->texture.height = height;
    field->texture.data = distanceFieldTexData;
    createTexture(&field->texture, GL_RED);

    popArray<uint8>(arena, width * height);
}

static void flood(real32* field, real32 level, int32* regionIds_t0, int32* regionIds_t1,
    uint32 width, uint32 height,
    uint32 i, uint32 j, int32 newId)
{
    // top
    if (j + 1 < height) {
        uint32 n = i + (j + 1) * width;
        if (field[n] < level && regionIds_t0[n] == -1) {
            regionIds_t1[n] = newId;
            regionIds_t0[n] = newId;
            flood(field, level, regionIds_t0, regionIds_t1, width, height, i, j + 1, newId);
        }
    }
    // right
    if (i + 1 < width) {
        uint32 n = i + 1 + j * width;
        if (field[n] < level && regionIds_t0[n] == -1) {
            regionIds_t1[n] = newId;
            regionIds_t0[n] = newId;
            flood(field, level, regionIds_t0, regionIds_t1, width, height, i + 1, j, newId);
        }
    }
    // bottom
    if (j - 1 < height) {
        uint32 n = i + (j - 1) * width;
        if (field[n] < level && regionIds_t0[n] == -1) {
            regionIds_t1[n] = newId;
            regionIds_t0[n] = newId;
            flood(field, level, regionIds_t0, regionIds_t1, width, height, i, j - 1, newId);
        }
    }
    // left
    if (i - 1 < width) {
        uint32 n = i - 1 + j * width;
        if (field[n] < level && regionIds_t0[n] == -1) {
            regionIds_t1[n] = newId;
            regionIds_t0[n] = newId;
            flood(field, level, regionIds_t0, regionIds_t1, width, height, i - 1, j, newId);
        }
    }
};

static void floodNew(real32* field, real32 level, int32* regionIds_t0, int32* regionIds_t1,
    uint32 width, uint32 height,
    uint32 i, uint32 j, int32 newId)
{
    // top
    if (j + 1 < height) {
        uint32 n = i + (j + 1) * width;
        if (field[n] < level && regionIds_t0[n] == -1 && regionIds_t1[n] == -1) {
            regionIds_t1[n] = newId;
            floodNew(field, level, regionIds_t0, regionIds_t1, width, height, i, j + 1, newId);
        }
    }
    // right
    if (i + 1 < width) {
        uint32 n = i + 1 + j * width;
        if (field[n] < level && regionIds_t0[n] == -1 && regionIds_t1[n] == -1) {
            regionIds_t1[n] = newId;
            floodNew(field, level, regionIds_t0, regionIds_t1, width, height, i + 1, j, newId);
        }
    }
    // bottom
    if (j - 1 < height) {
        uint32 n = i + (j - 1) * width;
        if (field[n] < level && regionIds_t0[n] == -1 && regionIds_t1[n] == -1) {
            regionIds_t1[n] = newId;
            floodNew(field, level, regionIds_t0, regionIds_t1, width, height, i, j - 1, newId);
        }
    }
    // left
    if (i - 1 < width) {
        uint32 n = i - 1 + j * width;
        if (field[n] < level && regionIds_t0[n] == -1 && regionIds_t1[n] == -1) {
            regionIds_t1[n] = newId;
            floodNew(field, level, regionIds_t0, regionIds_t1, width, height, i - 1, j, newId);
        }
    }
};

void genRegions(MemoryArena* arena, DistanceField* distanceField, RegionIdMap* regions)
{

    uint32 width = distanceField->width;
    uint32 height = distanceField->height;
    size_t count = width * height;

    int32* regionIds_t0 = pushArray<int32>(arena, count);
    int32* regionIds_t1 = pushArray<int32>(arena, count);

    regions->width = width;
    regions->height = height;
    regions->ids = regionIds_t0;

    real32 levelValue = distanceField->minVal;
    real32 step = 0.1;
    int32 nextId = 0;

    for (uint32 j = 0; j < height; j++) {
        for (uint32 i = 0; i < width; i++) {
            regionIds_t0[i + j * width] = -1;
        }
    }

    while (levelValue < 0.0f) {
        for (uint32 j = 0; j < count; j++) {
            regionIds_t1[j] = -1;
        }

        // Flood the existing regions.
        for (uint32 j = 0; j < height; j++) {
            for (uint32 i = 0; i < width; i++) {
                uint32 v = i + j * width;
                if (distanceField->field[v] < levelValue) {
                    if (regionIds_t0[v] != -1) {
                        regionIds_t1[v] = regionIds_t0[v];
                        // flood neighboring pixels
                        flood(distanceField->field, levelValue, regionIds_t0, regionIds_t1, width, height,
                            i, j, regionIds_t0[v]);
                    }
                }
            }
        }

        // Flood newly created regions.
        for (uint32 j = 0; j < height; j++) {
            for (uint32 i = 0; i < width; i++) {
                uint32 v = i + j * width;
                if (distanceField->field[v] < levelValue) {
                    if (regionIds_t0[v] == -1 && regionIds_t1[v] == -1) {
                        // Create a new region.
                        regionIds_t1[v] = nextId++;
                        // flood neighboring pixels
                        floodNew(distanceField->field, levelValue, regionIds_t0, regionIds_t1, width, height,
                            i, j, regionIds_t1[v]);
                    }
                }
            }
        }

        std::swap(regionIds_t0, regionIds_t1);

        levelValue += step;
    }

    regions->regionCount = nextId;

    std::function<int32(uint32, uint32)> mostImportantNeighbor = [&](uint32 i, uint32 j) -> int32 {
        struct IdMap {
            int32 neighboors[4];
            int32 counts[4];
            int32 ids = 0;
        };
        const auto increment = [&](IdMap* map, int32 id) {
            for (int32 i = 0; i < map->ids; i++) {
                if (map->neighboors[i] == id) {
                    map->counts[i]++;
                    return;
                }
            }
            map->neighboors[map->ids] = id;
            map->counts[map->ids] = 1;
            map->ids++;
        };

        IdMap map;
        // top
        if (j + 1 < height)
            increment(&map, regionIds_t0[i + (j + 1) * width]);
        // right
        if (i + 1 < width)
            increment(&map, regionIds_t0[i + 1 + j * width]);
        // bottom
        if (j > 0)
            increment(&map, regionIds_t0[i + (j - 1) * width]);
        // left
        if (i > 0)
            increment(&map, regionIds_t0[i - 1 + j * width]);

        const auto maximum = [&](IdMap* map) {
            int32 maxI = -1;
            int32 maxC = -1;
            for (int32 i = 0; i < map->ids; i++) {
                if (map->counts[i] > maxC) {
                    maxC = map->counts[i];
                    maxI = i;
                }
            }
            return map->neighboors[maxI];
        };

        return maximum(&map);
    };

    std::function<void(uint32, uint32, int32)> mergeIfIsolated = [&](uint32 i, uint32 j, int32 gid) {
        if (j >= height || i >= width)
            return;
        if (regionIds_t0[i + j * width] != gid)
            return;

        int32 c = 0;
        int32 id = regionIds_t0[i + j * width];
        // top
        if (j + 1 < height) {
            uint32 n = i + (j + 1) * width;
            if (regionIds_t0[n] == id)
                c++;
        }
        // right
        if (i + 1 < width) {
            uint32 n = i + 1 + j * width;
            if (regionIds_t0[n] == id)
                c++;
        }
        // bottom
        if (j - 1 < height) {
            uint32 n = i + (j - 1) * width;
            if (regionIds_t0[n] == id)
                c++;
        }
        // left
        if (i - 1 < width) {
            uint32 n = i - 1 + j * width;
            if (regionIds_t0[n] == id)
                c++;
        }

        if (c < 2) {
            regionIds_t0[i + j * width] = mostImportantNeighbor(i, j);
            mergeIfIsolated(i, j + 1, gid);
            mergeIfIsolated(i + 1, j, gid);
            mergeIfIsolated(i, j - 1, gid);
            mergeIfIsolated(i - 1, j, gid);
        }
    };

    // We don't want cells with only one neighboor of the same id.
    for (uint32 j = 0; j < height; j++) {
        for (uint32 i = 0; i < width; i++) {
            if (regionIds_t0[i + j * width] != -1) {
                mergeIfIsolated(i, j, regionIds_t0[i + j * width]);
            }
        }
    }

    // If a regions wrap around a non walkable region, these non walkable region will become walkable.
    // So we go through unwalkable regions, and if they have only one region as neighboor, split this region.

    // Assign a random color to each id
    RGB* colors = pushArray<RGB>(arena, nextId);
    for (int32 i = 0; i < nextId; i++) {
        colors[i].r = 255 * (real32)rand() / RAND_MAX;
        colors[i].g = 255 * (real32)rand() / RAND_MAX;
        colors[i].b = 255 * (real32)rand() / RAND_MAX;
    }

    // Write a segment texture with id->color map.
    RGBA* segmentsTexData = pushArray<RGBA>(arena, count);
    for (uint32 j = 0; j < height; j++) {
        for (uint32 i = 0; i < width; i++) {
            uint32 index = i + j * width;
            if (regionIds_t0[index] != -1) {
                uint32 cId = regionIds_t0[index];
                RGB col = colors[cId];
                segmentsTexData[index] = { col.r, col.g, col.b, 255 };
            }
            else {
                segmentsTexData[index] = { 0, 0, 0, 0 };
            }
        }
    }

    regions->lastId = nextId;
    regions->texture.width = width;
    regions->texture.height = height;
    regions->texture.data = segmentsTexData;
    createTexture(&regions->texture, GL_RGBA);

    // colors
    popArray<RGBA>(arena, count);
    // segmentTexData
    popArray<RGB>(arena, nextId);
    // regionIds_t1
    popArray<int32>(arena, count);
}

static int32 idAt(int32* ids, uint32 w, uint32 i, uint32 j)
{
    return ids[i + j * w];
};

enum Dir {
    UP = 0,
    RIGHT,
    DOWN,
    LEFT
};

static int32 idAtRight(int32* ids, uint32 w, uint32 h, uint32 i, uint32 j, Dir dir)
{
    switch (dir) {
    case UP:
        return j == 0 ? -1 : idAt(ids, w, i, j - 1);
    case RIGHT:
        return i + 1 >= w ? -1 : idAt(ids, w, i + 1, j);
    case DOWN:
        return j + 1 >= h ? -1 : idAt(ids, w, i, j + 1);
    case LEFT:
        return i == 0 ? -1 : idAt(ids, w, i - 1, j);
    }
    return -1;
};

static bool isEdge(int32* ids, uint32 w, uint32 h, uint32 i, uint32 j, Dir dir)
{
    switch (dir) {
    case UP:
        return j == 0 || idAt(ids, w, i, j) != idAt(ids, w, i, j - 1);
    case RIGHT:
        return i + 1 >= w || idAt(ids, w, i, j) != idAt(ids, w, i + 1, j);
    case DOWN:
        return j + 1 >= h || idAt(ids, w, i, j) != idAt(ids, w, i, j + 1);
    case LEFT:
        return i == 0 || idAt(ids, w, i, j) != idAt(ids, w, i - 1, j);
    }
    return false;
};

void genContours(MemoryArena* arena, RegionIdMap* regions, ContourSet* contours)
{

    uint32 w = regions->width;
    uint32 h = regions->height;
    int32* ids = regions->ids;

    contours->count = 0;
    contours->contours = pushArray<Contour>(arena, regions->regionCount);

    bool* visited = pushArray<bool>(arena, regions->lastId);
    for (int32 i = 0; i < regions->lastId; i++) {
        visited[i] = false;
    }

    // Visit all cell to find beginings of contours, mark visited contour.
    for (uint32 j = 0; j < h; j++) {
        for (uint32 i = 0; i < w; i++) {

            int32 cellId = ids[i + j * w];
            if (cellId == -1 || visited[cellId]) {
                continue;
            }
            // Check if this is a border cell to start a region
            if (!isEdge(ids, w, h, i, j, UP)
                && !isEdge(ids, w, h, i, j, RIGHT)
                && !isEdge(ids, w, h, i, j, DOWN)
                && !isEdge(ids, w, h, i, j, LEFT)) {
                continue;
            }
            visited[cellId] = 1;

            uint32 count = 0;
            Vec3* verts = pushArray<Vec3>(arena, w * h);

            // We visit in order from top-left to bottom so the direction must be DOWN for ccw order.
            Dir fDir = DOWN;
            Dir rDir = (Dir)((fDir + 1) % 4);

            uint32 x = i;
            uint32 y = j;
            uint32 xb = i;
            uint32 yb = j;

            do {
                int32 idRight = idAtRight(ids, w, h, x, y, rDir);
                bool endOfSegment = false;
                bool edgeForward;
                bool edgeRight;

                // Go to the end of the segment.
                do {
                    switch (fDir) {
                    case UP:
                        y--;
                        break;
                    case RIGHT:
                        x++;
                        break;
                    case DOWN:
                        y++;
                        break;
                    case LEFT:
                        x--;
                        break;
                    default:
                        break;
                    }
                    int32 newIdRight = idAtRight(ids, w, h, x, y, rDir);
                    edgeForward = isEdge(ids, w, h, x, y, fDir);
                    edgeRight = isEdge(ids, w, h, x, y, rDir);

                    endOfSegment = edgeForward || !edgeRight;

                    if (edgeRight && newIdRight != idRight) {
                        Vec3 cellCenter = Vec3(x + 0.5, h - y - 0.5, 0.5);
                        // Add a vertex backwards.
                        Vec3 offset;
                        switch (fDir) {
                        case UP:
                            offset = Vec3(.5f, -.5f, .0f);
                            break;
                        case RIGHT:
                            offset = Vec3(-.5f, -.5f, .0f);
                            break;
                        case DOWN:
                            offset = Vec3(-.5f, .5f, .0f);
                            break;
                        case LEFT:
                            offset = Vec3(.5f, .5f, .0f);
                            break;
                        }

                        verts[count++] = cellCenter + offset;
                        idRight = newIdRight;
                    }
                } while (!endOfSegment);

                // Add a vertex.
                if (edgeForward && edgeRight) {
                    ASSERT(count < w * h);
                    Vec3 cellCenter = Vec3(x + 0.5, h - y - 0.5, 0.5);
                    Vec3 offset;
                    // forwards.
                    switch (fDir) {
                    case UP:
                        offset = Vec3(.5f, .5f, .0f);
                        break;
                    case RIGHT:
                        offset = Vec3(.5f, -.5f, .0f);
                        break;
                    case DOWN:
                        offset = Vec3(-.5f, -.5f, .0f);
                        break;
                    case LEFT:
                        offset = Vec3(-.5f, .5f, .0f);
                        break;
                    }
                    verts[count++] = cellCenter + offset;
                }
                // Turn
                if (!edgeRight) {
                    // turn right.
                    fDir = rDir;
                    rDir = (Dir)((fDir + 1) % 4);
                }
                else {
                    // turn left.
                    rDir = fDir;
                    fDir = fDir == 0 ? LEFT : (Dir)((fDir - 1) % 4);
                }
            } while (x != xb || y != yb);

            // If the start position is in a 1 cell wide limb, add a vertex.
            // we can spot that if the end direction is not the same as the start direction
            if (fDir != DOWN) {
                ASSERT(fDir == LEFT);
                Vec3 cellCenter = Vec3(x + 0.5, h - y - 0.5, 0.5);
                verts[count++] = cellCenter + Vec3(-.5f, .5f, .0f);
            }

            // resize verts
            popArray<Vec3>(arena, w * h - count);

            Contour* contour = &contours->contours[contours->count++];
            contour->vertices = verts;
            contour->count = count;
        }
    }
}

static uint32 nextFree(bool* excluded, uint32 count, uint32 start)
{
    uint32 i = start;
    while (excluded[i])
        i = (i + 1) % count;
    return i;
}

static bool isCCWOrColinear(const Vec3& p0, const Vec3& p1, const Vec3& p2)
{
    return cross(p1 - p0, p2 - p1).z >= 0;
}

void triangulateContours(MemoryArena* arena, ContourSet* contours, Mesh3D* meshes)
{

    uint32 totalVCount = 0;
    uint32 totalFCount = 0;
    for (uint32 c = 0; c < contours->count; c++) {
        totalVCount += contours->contours[c].count;
        totalFCount += contours->contours[c].count - 2;
    }

    uint32* gindices = pushArray<uint32>(arena, 3 * totalFCount);
    Vec3* gvertices = contours->contours[0].vertices;
    uint32* indices = gindices;

    uint32 cVCount = 0;
    uint32 cFCount = 0;

    for (uint32 c = 0; c < contours->count; c++) {
        Mesh3D* mesh = meshes + c;
        memset(mesh, 0, sizeof(Mesh3D));

        Contour* contour = contours->contours + c;
        Vec3* verts = contour->vertices;
        ASSERT(verts == gvertices + cVCount);

        uint32 vCount = contour->count;
        uint32 fCount = vCount - 2;

        mesh->positions = verts;
        mesh->indices = indices;
        mesh->vCount = vCount;
        mesh->fCount = fCount;

        cVCount += vCount;
        cFCount += fCount;

        bool* excluded = pushArrayZeroed<bool>(arena, vCount);

        for (uint32 f = 0; f < fCount; f++) {

            real32 minEdgeLength = 1000000;
            uint32 mi0, mi1, mi2;

            bool found = false;
            uint32 l = 0;

            for (uint32 i = 0; i < vCount - f; i++) {
                uint32 i0 = nextFree(excluded, vCount, l);
                uint32 i1 = nextFree(excluded, vCount, (i0 + 1) % vCount);
                uint32 i2 = nextFree(excluded, vCount, (i1 + 1) % vCount);

                if (isCCW(verts[i0], verts[i1], verts[i2])) {
                    real32 eLength = length(verts[i2] - verts[i0]);
                    if (eLength < minEdgeLength) {
                        found = true;
                        minEdgeLength = eLength;
                        mi0 = i0;
                        mi1 = i1;
                        mi2 = i2;
                    }
                }
                l = i1;
            }
            ASSERT(found);
            excluded[mi1] = true;
            indices[3 * f] = mi0;
            indices[3 * f + 1] = mi1;
            indices[3 * f + 2] = mi2;

            ASSERT(isCCW(verts[mi0], verts[mi1], verts[mi2]));
        }
        indices += 3 * fCount;
        popArray<bool>(arena, vCount);
    }
}

static uint32 findOrAddVertex(NavMesh* mesh, const Vec3& pos)
{
    // Linear search, FIXME: implement hash map.
    Vec3* verts = mesh->vertices;
    for (uint32 i = 0; i < mesh->vertCount; i++) {
        if (pos.x == verts[i].x && pos.y == verts[i].y && pos.z == verts[i].z) {
            return i;
        }
    }

    // Not found, add at the end.
    uint32 i = mesh->vertCount++;
    verts[i] = pos;
    return i;
}

static uint32 polyVertCount(uint32 mvp, uint32* poly)
{
    uint32 pvCount = 0;
    while (poly[pvCount] != NULL_INDEX && pvCount < mvp) {
        pvCount++;
    }
    return pvCount;
}

static real32 validMergeEdgeLength(NavMesh* mesh, uint32* p, uint32* q, uint32& ve, uint32& we)
{

    uint32 pvCount = polyVertCount(mesh->maxVertPerPoly, p);
    uint32 qvCount = polyVertCount(mesh->maxVertPerPoly, q);

    if (pvCount + qvCount - 2 > mesh->maxVertPerPoly) {
        return 0;
    }

    // For each edge
    for (uint32 i = 0; i < pvCount; i++) {
        uint32 vp = p[i];
        uint32 j = next(i, pvCount);
        uint32 wp = p[j];

        for (uint32 l = 0; l < qvCount; l++) {
            // the edge is shared.
            uint32 wq = q[l];
            uint32 k = prev(l, qvCount);
            uint32 vq = q[k];
            if (wq == vp && vq == wp) {

                // If the resulting merge is convex.
                Vec3 pVPrevP = mesh->vertices[p[prev(i, pvCount)]];
                Vec3 pV = mesh->vertices[vp];
                Vec3 pVNextQ = mesh->vertices[q[next(l, qvCount)]];

                Vec3 pWPrevQ = mesh->vertices[q[prev(k, qvCount)]];
                Vec3 pW = mesh->vertices[wp];
                Vec3 pWNextP = mesh->vertices[p[next(j, pvCount)]];

                if (isCCWOrColinear(pVPrevP, pV, pVNextQ) && isCCWOrColinear(pWPrevQ, pW, pWNextP)) {
                    ve = j;
                    we = l;
                    return sqrLength(pV - pW);
                }
                else
                    return 0.0;
            }
        }
    }

    return 0;
}

static void mergePolygons(MemoryArena* arena, uint32* polys, uint32 nPolys, uint32 mvp, uint32 p, uint32 q, uint32 pb, uint32 qb)
{

    uint32* polyP = polys + p * mvp;
    uint32* polyQ = polys + q * mvp;

    uint32 pvCount = polyVertCount(mvp, polyP);
    ASSERT(pvCount >= 3);
    uint32 qvCount = polyVertCount(mvp, polyQ);
    ASSERT(qvCount >= 3);

    uint32* temp = pushArray<uint32>(arena, mvp);
    memcpy(temp, polyP, mvp * sizeof(uint32));
    memset(polyP, 0xff, mvp * sizeof(uint32));

    // Copy indices from P
    uint32 pi = pb;
    for (uint32 i = 0; i < pvCount - 1; i++) {
        polyP[i] = temp[pi];
        pi = next(pi, pvCount);
    }

    // Copy indices from Q.
    uint32 qi = qb;
    for (uint32 i = 0; i < qvCount - 1; i++) {
        polyP[i + pvCount - 1] = polyQ[qi];
        qi = next(qi, qvCount);
    }

    if (q != nPolys - 1) {
        memcpy(polyQ, polys + mvp * (nPolys - 1), mvp * sizeof(uint32));
    }
    popArray<uint32>(arena, mvp);
}

struct Edge {
    uint32 i1;
    uint32 i0p;
    uint32 i1p;
    uint32 f0;
    uint32 f1;
    Edge* next;
    bool notBorder;
};

static Vec3 polygonCenter(Vec3* verts, uint32* poly, uint32 mvp)
{
    Vec3 p = { 0, 0, 0 };
    uint32 count = 0;
    while (count < mvp && poly[count] != NULL_INDEX) {
        p += verts[poly[count++]];
    }
    return (1.0f / count) * p;
}

void buildDualMesh(MemoryArena* arena, NavMesh* mesh,
    Edge* edges, uint32 edgeCount, DualMesh* dual)
{
    Vec3* verts = pushArray<Vec3>(arena, mesh->polyCount);
    Vec3* mVerts = mesh->vertices;
    uint32 mvp = mesh->maxVertPerPoly;

    for (uint32 p = 0; p < mesh->polyCount; p++) {
        uint32* poly = mesh->polygons + 2 * p * mvp;
        verts[p] = polygonCenter(mVerts, poly, mvp);
    }

    uint32* indices = pushArray<uint32>(arena, 2 * edgeCount);
    uint32 iCount = 0;
    for (uint32 e = 0; e < edgeCount; e++) {
        Edge* edge = edges + e;
        if (edge->notBorder) {
            indices[iCount++] = edge->f0;
            indices[iCount++] = edge->f1;
        }
    }

    popArray<uint32>(arena, 2 * edgeCount - iCount);

    mesh->polyCenters = verts;

    dual->vertices = verts;
    dual->vertCount = mesh->polyCount;
    dual->indices = indices;
    dual->indCount = iCount;
}

static void computePolygonAdjacency(MemoryArena* arena, NavMesh* mesh, DualMesh* dual)
{

    // An edge has 2 vertex indices i0, i1.
    // 2 polygons share an edge if one has (i0, i1) and the other (i1, i0).
    // We first construct the set of edges such that i0 < i1 and,
    // for each vertex i, create a list of the edges such that i is the first vertex of the edge,
    // then for each edge (i0, i1) such that i0 > i1, we can find the oposite edge if it exists in the list of i1.

    uint32 mvp = mesh->maxVertPerPoly;

    // list of edges (i,j) whose first vertex is i and for which i < j.
    Edge* edges = pushArrayZeroed<Edge>(arena, mesh->polyCount * mvp);
    uint32 edgeCount = 0;
    Edge** edgesFromVertex = pushArrayZeroed<Edge*>(arena, mesh->vertCount);

    for (uint32 p = 0; p < mesh->polyCount; p++) {
        uint32 i = 0;
        uint32* poly = mesh->polygons + 2 * p * mvp;
        while (poly[i] != NULL_INDEX && i < mvp) {
            uint32 i0 = poly[i];
            uint32 i1 = poly[next(i, mvp)];
            if (i1 == NULL_INDEX)
                i1 = poly[0];

            if (i0 < i1) {
                Edge* edge = edges + edgeCount++;
                edge->i0p = i;
                edge->i1 = i1;
                edge->f0 = p;

                Edge* tail = edgesFromVertex[i0];
                if (tail == NULL) {
                    edgesFromVertex[i0] = edge;
                }
                else {
                    while (tail->next != NULL) {
                        tail = tail->next;
                    }
                    tail->next = edge;
                }
            }
            i++;
        }
    }

    for (uint32 p = 0; p < mesh->polyCount; p++) {
        uint32 i = 0;
        uint32* poly = mesh->polygons + 2 * p * mvp;
        while (poly[i] != NULL_INDEX && i < mvp) {
            uint32 i1 = poly[i];
            uint32 i0 = poly[next(i, mvp)];
            if (i0 == NULL_INDEX)
                i0 = poly[0];

            if (i0 < i1) {
                Edge* edge = edgesFromVertex[i0];
                while (edge != NULL && edge->i1 != i1 && edge->next != NULL) {
                    edge = edge->next;
                }

                if (edge != NULL && edge->i1 == i1) {
                    edge->f1 = p;
                    edge->i1p = i;
                    edge->notBorder = true;
                }
            }
            i++;
        }
    }

    for (uint32 e = 0; e < edgeCount; e++) {
        Edge* edge = edges + e;
        if (!edge->notBorder) {
            continue;
        }

        uint32* p0 = mesh->polygons + 2 * edge->f0 * mvp;
        uint32* p1 = mesh->polygons + 2 * edge->f1 * mvp;

        p0[mvp + edge->i0p] = edge->f1;
        p1[mvp + edge->i1p] = edge->f0;
    }

    buildDualMesh(arena, mesh, edges, edgeCount, dual);

    popArray<Edge*>(arena, mesh->vertCount);
    popArray<Edge>(arena, mesh->polyCount * mvp);
}

void buildNavMesh(MemoryArena* arena, ContourSet* contours, Mesh3D* triMeshes, NavMesh* mesh, DualMesh* dual)
{

    // Convert tri meshes to poly meshes. Each mesh is converted to a polygon and all polygons share the same vertices.
    uint32 totalVCount = 0;
    uint32 totalTriCount = 0;
    for (uint32 c = 0; c < contours->count; c++) {
        totalVCount += contours->contours[c].count;
        totalTriCount += contours->contours[c].count - 2;
    }

    mesh->maxVertPerPoly = 6;
    uint32 maxVertsPerPoly = mesh->maxVertPerPoly;

    // Vertices of the final mesh without duplication.
    Vec3* meshVerts = pushArray<Vec3>(arena, totalVCount);
    uint32* meshPolys = pushArray<uint32>(arena, 2 * maxVertsPerPoly * totalTriCount, NULL_INDEX);
    mesh->vertCount = 0;
    mesh->vertices = meshVerts;
    mesh->polyCount = 0;
    mesh->polygons = meshPolys;

    for (uint32 c = 0; c < contours->count; c++) {
        Mesh3D* triMesh = triMeshes + c;
        Vec3* triVerts = triMesh->positions;
        uint32* triInds = triMesh->indices;

        // Index mapping because we are merging vertices.
        uint32* iMap = pushArray<uint32>(arena, triMesh->vCount);
        for (uint32 v = 0; v < triMesh->vCount; v++) {
            iMap[v] = findOrAddVertex(mesh, triVerts[v]);
        }

        // Temporary polygon soup intialized with the triangles.
        uint32* polys = pushArray<uint32>(arena, maxVertsPerPoly * triMesh->fCount);
        memset(polys, 0xff, maxVertsPerPoly * triMesh->fCount * sizeof(uint32));
        for (uint32 t = 0; t < triMesh->fCount; t++) {
            uint32* poly = polys + maxVertsPerPoly * t;
            uint32* tri = triInds + 3 * t;
            poly[0] = iMap[tri[0]];
            poly[1] = iMap[tri[1]];
            poly[2] = iMap[tri[2]];
        }
        uint32 nPolys = triMesh->fCount;

        // Merge Polygons.
        for (;;) {
            // Find p0 and p1, polygons with longest shared edge.
            uint32 p0, p1;
            uint32 v0, v1;
            real32 maxEdge = .0f;

            for (uint32 p = 0; p < nPolys - 1; p++) {
                uint32* polyP = polys + maxVertsPerPoly * p;
                for (uint32 q = p + 1; q < nPolys; q++) {
                    // if p and q share an edge and are a valid convex merge.
                    uint32* polyQ = polys + maxVertsPerPoly * q;
                    uint32 v, w;
                    real32 e = validMergeEdgeLength(mesh, polyP, polyQ, v, w);

                    if (e > maxEdge) {
                        p0 = p;
                        p1 = q;
                        v0 = v;
                        v1 = w;
                        maxEdge = e;
                    }
                }
            }
            // If we can merge, do it
            if (maxEdge > 0) {
                mergePolygons(arena, polys, nPolys, maxVertsPerPoly, p0, p1, v0, v1);

                // Check that the poly is ok.
                uint32* poly = polys + maxVertsPerPoly * p0;
                uint32 c = polyVertCount(maxVertsPerPoly, poly);
                ASSERT(c >= 3);

                nPolys--;
            }
            else {
                // Stop if no possible merge was found.
                break;
            }
        }

        // Copy Polygons to NavMesh.
        for (uint32 p = 0; p < nPolys; p++) {
            uint32* mPoly = mesh->polygons + 2 * maxVertsPerPoly * mesh->polyCount++;
            uint32* poly = polys + maxVertsPerPoly * p;
            for (uint32 i = 0; i < maxVertsPerPoly; i++) {
                mPoly[i] = poly[i];
            }
            ASSERT(polyVertCount(mesh, p) >= 3);
        }

        popArray<uint32>(arena, maxVertsPerPoly * triMesh->fCount);
        popArray<uint32>(arena, triMeshes->vCount);
    }

    computePolygonAdjacency(arena, mesh, dual);
}

uint32 polyVertCount(NavMesh* mesh, uint32 polyRef)
{
    ASSERT(polyRef < mesh->polyCount);
    uint32* poly = mesh->polygons + 2 * polyRef * mesh->maxVertPerPoly;
    return polyVertCount(mesh->maxVertPerPoly, poly);
}

bool checkNavMesh(NavMesh* mesh)
{
    for (uint32 i = 0; i < mesh->polyCount; i++) {
        ASSERT(polyVertCount(mesh, i) >= 3);
    }
    return true;
}
