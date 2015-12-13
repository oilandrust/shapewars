//
//  NavMeshGen.cpp
//  GetTheFlag
//
//  Created by Olivier on 05/12/15.
//
//

#include "NavMeshGen.h"

void genDistanceField(MemoryArena* arena, Level* level, DistanceField* field)
{
    field->width = level->width;
    field->height = level->height;
    
    uint8* levelMap = level->tiles;
    
    uint32 width = field->width;
    uint32 height = field->height;
    size_t count = width*height;
    
    real32* distanceField = pushArray<real32>(arena, count);
    uint8* distanceFieldTexData = pushArray<uint8>(arena, count);
    field->field = distanceField;
    
    // Retrieve the position of the walls.
    uint32 wallCount = 0;
    for(uint32 j = 0; j < count; j++) {
        if(levelMap[j]) wallCount++;
    }
    Vec2* walls = pushArray<Vec2>(arena, wallCount);
    uint32 w = 0;
    for(uint32 j = 0; j < height; j++) {
        for(uint32 i = 0; i < width; i++) {
            if(levelMap[i+j*width]) walls[w++] = Vec2(i, j);
        }
    }

    // Compute the distance field.
    real32 maxDist = 0;
    for(uint32 j = 0; j < height; j++) {
        for(uint32 i = 0; i < width; i++) {
            Vec2 pt(i, j);
            real32 minDist = 10000000;
            for(uint32 w = 0; w < wallCount; w++) {
                real32 dist = length(pt-walls[w]);
                if(dist < minDist) {
                    minDist = dist;
                }
            }
            distanceField[i+j*width] = -minDist;
            if(minDist > maxDist) {
                maxDist = minDist;
            }
        }
    }
    field->minVal = -maxDist;

    popArray<Vec2>(arena, wallCount);
    
    // Update the texture data.
    for(uint32 j = 0; j < height; j++) {
        for(uint32 i = 0; i < width; i++) {
            distanceFieldTexData[i+j*width] = -255*distanceField[i+j*width] / maxDist;
        }
    }
    field->texture.width = width;
    field->texture.height = height;
    field->texture.data = distanceFieldTexData;
    createTexture(&field->texture, GL_RED);
    
    popArray<uint8>(arena, width*height);
}

void flood(real32* field, real32 level, int32* regionIds_t0, int32* regionIds_t1,
           uint32 width, uint32 height,
           uint32 i, uint32 j, int32 newId) {
    // top
    if(j+1 < height) {
        uint32 n = i+(j+1)*width;
        if(field[n] < level && regionIds_t0[n] == -1) {
            regionIds_t1[n] = newId;
            regionIds_t0[n] = newId;
            flood(field, level, regionIds_t0, regionIds_t1, width, height, i, j+1, newId);
        }
    }
    // right
    if(i+1 < width) {
        uint32 n = i+1+j*width;
        if(field[n] < level && regionIds_t0[n] == -1) {
            regionIds_t1[n] = newId;
            regionIds_t0[n] = newId;
            flood(field, level, regionIds_t0, regionIds_t1, width, height, i+1, j, newId);
        }
    }
    // bottom
    if(j-1 < height) {
        uint32 n = i+(j-1)*width;
        if(field[n] < level && regionIds_t0[n] == -1) {
            regionIds_t1[n] = newId;
            regionIds_t0[n] = newId;
            flood(field, level, regionIds_t0, regionIds_t1, width, height, i, j-1, newId);
        }
    }
    // left
    if(i-1 < width) {
        uint32 n = i-1+j*width;
        if(field[n] < level && regionIds_t0[n] == -1) {
            regionIds_t1[n] = newId;
            regionIds_t0[n] = newId;
            flood(field, level, regionIds_t0, regionIds_t1, width, height, i-1, j, newId);
        }
    }
};


void floodNew(real32* field, real32 level, int32* regionIds_t0, int32* regionIds_t1,
           uint32 width, uint32 height,
           uint32 i, uint32 j, int32 newId) {
    // top
    if(j+1 < height) {
        uint32 n = i+(j+1)*width;
        if(field[n] < level && regionIds_t0[n] == -1 && regionIds_t1[n] == -1) {
            regionIds_t1[n] = newId;
            floodNew(field, level, regionIds_t0, regionIds_t1, width, height, i, j+1, newId);
        }
    }
    // right
    if(i+1 < width) {
        uint32 n = i+1+j*width;
        if(field[n] < level && regionIds_t0[n] == -1 && regionIds_t1[n] == -1) {
            regionIds_t1[n] = newId;
            floodNew(field, level, regionIds_t0, regionIds_t1, width, height, i+1, j, newId);
        }
    }
    // bottom
    if(j-1 < height) {
        uint32 n = i+(j-1)*width;
        if(field[n] < level && regionIds_t0[n] == -1 && regionIds_t1[n] == -1) {
            regionIds_t1[n] = newId;
            floodNew(field, level, regionIds_t0, regionIds_t1, width, height, i, j-1, newId);
        }
    }
    // left
    if(i-1 < width) {
        uint32 n = i-1+j*width;
        if(field[n] < level && regionIds_t0[n] == -1 && regionIds_t1[n] == -1) {
            regionIds_t1[n] = newId;
            floodNew(field, level, regionIds_t0, regionIds_t1, width, height, i-1, j, newId);
        }
    }
};


void genRegions(MemoryArena* arena, DistanceField* distanceField, RegionIdMap* regions) {
    
    uint32 width = distanceField->width;
    uint32 height = distanceField->height;
    size_t count = width*height;
    
    int32* regionIds_t0 = pushArray<int32>(arena, count);
    int32* regionIds_t1 = pushArray<int32>(arena, count);
    
    regions->width = width;
    regions->height = height;
    regions->ids = regionIds_t0;
    
    real32 levelValue = distanceField->minVal;
    real32 step = 0.1;
    int32 nextId = 0;
    
    for(uint32 j = 0; j < height; j++) {
        for(uint32 i = 0; i < width; i++) {
            regionIds_t0[i+j*width] = -1;
        }
    }
    
    while(levelValue < 0.f) {
        for(uint32 j = 0; j < count; j++) {
            regionIds_t1[j] = -1;
        }
        
        // Flood the existing regions.
        for(uint32 j = 0; j < height; j++) {
            for(uint32 i = 0; i < width; i++) {
                uint32 v = i+j*width;
                if(distanceField->field[v] < levelValue) {
                    if(regionIds_t0[v] != -1) {
                        regionIds_t1[v] = regionIds_t0[v];
                        // flood neighboring pixels
                        flood(distanceField->field, levelValue, regionIds_t0, regionIds_t1, width, height,
                              i, j, regionIds_t0[v]);
                    }
                }
            }
        }
        
        // Flood newly created regions.
        for(uint32 j = 0; j < height; j++) {
            for(uint32 i = 0; i < width; i++) {
                uint32 v = i+j*width;
                if(distanceField->field[v] < levelValue) {
                    if(regionIds_t0[v] == -1 && regionIds_t1[v] == -1) {
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
    
    std::function<int32(uint32,uint32)> mostImportantNeighbor = [&](uint32 i, uint32 j) -> int32 {
        struct IdMap {
            int32 neighboors[4];
            int32 counts[4];
            int32 ids = 0;
        };
        const auto increment = [&](IdMap* map, int32 id) {
            for(int32 i = 0; i < map->ids; i++) {
                if(map->neighboors[i] == id) {
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
        if(j+1 < height) increment(&map, regionIds_t0[i+(j+1)*width]);
        // right
        if(i+1 < width) increment(&map, regionIds_t0[i+1+j*width]);
        // bottom
        if(j > 0) increment(&map, regionIds_t0[i+(j-1)*width]);
        // left
        if(i > 0) increment(&map, regionIds_t0[i-1+j*width]);
        
        const auto maximum = [&](IdMap* map) {
            int32 maxI=-1;
            int32 maxC=-1;
            for(int32 i = 0; i < map->ids; i++) {
                if(map->counts[i] > maxC) {
                    maxC = map->counts[i];
                    maxI = i;
                }
            }
            return map->neighboors[maxI];
        };
        
        return maximum(&map);
    };

    std::function<void(uint32,uint32,int32)> mergeIfIsolated = [&](uint32 i, uint32 j, int32 gid) {
        if(j >= height || i >= width) return;
        if(regionIds_t0[i+j*width] != gid) return;
            
        int32 c = 0;
        int32 id = regionIds_t0[i+j*width];
        // top
        if(j+1 < height) {
            uint32 n = i+(j+1)*width;
            if(regionIds_t0[n] == id) c++;
        }
        // right
        if(i+1 < width) {
            uint32 n = i+1+j*width;
            if(regionIds_t0[n] == id) c++;
        }
        // bottom
        if(j-1 < height) {
            uint32 n = i+(j-1)*width;
            if(regionIds_t0[n] == id) c++;
        }
        // left
        if(i-1 < width) {
            uint32 n = i-1+j*width;
            if(regionIds_t0[n] == id) c++;
        }
        
        if(c < 2) {
            regionIds_t0[i+j*width] = mostImportantNeighbor(i, j);
            mergeIfIsolated(i, j+1, gid);
            mergeIfIsolated(i+1, j, gid);
            mergeIfIsolated(i, j-1, gid);
            mergeIfIsolated(i-1, j, gid);
        }
    };
    
    // We don't want cells with only one neighboor of the same id.
    for(uint32 j = 0; j < height; j++) {
        for(uint32 i = 0; i < width; i++) {
            if(regionIds_t0[i+j*width] != -1) {
                mergeIfIsolated(i, j, regionIds_t0[i+j*width]);
            }
        }
    }
    
    // Assign a random color to each id
    RGB* colors = pushArray<RGB>(arena, nextId);
    for(int32 i = 0; i < nextId; i++) {
        colors[i].r = 255*(real32)rand()/RAND_MAX;
        colors[i].g = 255*(real32)rand()/RAND_MAX;
        colors[i].b = 255*(real32)rand()/RAND_MAX;
    }
    
    // Write a segment texture with id->color map.
    RGBA* segmentsTexData = pushArray<RGBA>(arena, count);
    for(uint32 j = 0; j < height; j++) {
        for(uint32 i = 0; i < width; i++) {
            uint32 index = i+j*width;
            if(regionIds_t0[index] != -1) {
                uint32 cId = regionIds_t0[index];
                RGB col = colors[cId];
                segmentsTexData[index] = {col.r, col.g, col.b, 255};
            } else {
                segmentsTexData[index] = {0,0,0,0};
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


int32 idAt(int32* ids, uint32 w, uint32 i, uint32 j) {
    return ids[i+j*w];
};

enum Dir {
    UP = 0,
    RIGHT,
    DOWN,
    LEFT
};

bool isEdge(int32* ids, uint32 w, uint32 h, uint32 i, uint32 j, Dir dir) {
    switch (dir) {
        case UP: return j == 0 || idAt(ids, w, i,j) != idAt(ids, w, i, j-1);
        case RIGHT: return i+1 >= w || idAt(ids, w, i,j) != idAt(ids, w, i+1, j);
        case DOWN: return j+1 >= h || idAt(ids, w, i,j) != idAt(ids, w, i, j+1);
        case LEFT: return i == 0 || idAt(ids, w, i,j) != idAt(ids, w, i-1, j);
    }
    return false;
};


void genContours(MemoryArena* arena, RegionIdMap* regions, Contours* contours) {
    
    uint32 w = regions->width;
    uint32 h = regions->height;
    int32* ids = regions->ids;
    
    contours->count = 0;
    contours->contours = pushArray<Contour>(arena, regions->regionCount);
    
    bool* visited = pushArray<bool>(arena, regions->lastId);
    for(int32 i = 0; i < regions->lastId; i++) {
        visited[i] = false;
    }
    
    // Visit all cell to find beginings of contours, mark visited contour.
    for(uint32 j = 0; j < h; j++) {
        for(uint32 i = 0; i < w; i++) {
            
            int32 cellId = ids[i+j*w];
            if(cellId == -1 || visited[cellId]) {
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
            Vec3* verts = pushArray<Vec3>(arena, w*h);

            // We visit in order from top-left to bottom so the direction must be DOWN for ccw order.
            Dir fDir = DOWN;
            Dir rDir = (Dir)((fDir+1)%4);
            
            // First vertex.
            verts[count++] = Vec3(i, h-j, 0.5);
            
            uint32 x = i;
            uint32 y = j;
            uint32 xb = i;
            uint32 yb = j;
            bool edgeForward;
            bool edgeLeft;
            do {
                // Go to the end of the segment.
                do {
                    switch (fDir) {
                        case UP: y--; break;
                        case RIGHT: x++; break;
                        case DOWN: y++; break;
                        case LEFT: x--; break;
                        default: break;
                    }
                    edgeForward = isEdge(ids, w, h, x, y, fDir);
                    edgeLeft = isEdge(ids, w, h, x, y, rDir);
                } while (!edgeForward && edgeLeft);
                
                // Add a vertex.
                ASSERT(count < w*h);
                Vec3 cellCenter = Vec3(x+0.5, h-y-0.5, 0.5);
                Vec3 offset(.0f,.0f,.0f);
                
                switch (fDir) {
                    case UP: offset = edgeForward?Vec3(.5f, .5f, .0f):Vec3(.5f, -.5f, .0f); break;
                    case RIGHT: offset = edgeForward?Vec3(.5f, -.5f, .0f):Vec3(-.5f, -.5f, .0f); break;
                    case DOWN: offset = edgeForward?Vec3(-.5f, -.5f, .0f):Vec3(-.5f, .5f, .0f); break;
                    case LEFT: offset = edgeForward?Vec3(-.5f, .5f, .0f):Vec3(.5f, .5f, .0f); break;
                }
                
                if(x != xb || y != yb) {
                    verts[count++] = cellCenter + offset;
                }
                // Turn
                if(isEdge(ids, w, h, x, y, fDir)) {
                    rDir = fDir;
                    fDir = fDir == 0?LEFT:(Dir)((fDir-1)%4);
                } else {
                    fDir = rDir;
                    rDir = (Dir)((fDir+1)%4);
                }
            } while(x != xb || y != yb);
            
            // resize verts
            popArray<Vec3>(arena, w*h - count);
            
            Contour* contour = &contours->contours[contours->count++];
            contour->vertices = verts;
            contour->count = count;
        }
    }
}

uint32 nextFree(bool* excluded, uint32 count, uint32 start) {
    uint32 i = start;
    while(excluded[i]) i = (i+1)%count;
    return i;
}

bool isCCW(const Vec3& p0, const Vec3& p1, const Vec3& p2) {
    return cross(p1-p0, p2-p1).z > 0;
}

void triangulateContours(MemoryArena* arena, Contours* contours, Mesh3D* meshes) {
    
    uint32 totalVCount = 0;
    uint32 totalFCount = 0;
    for(uint32 c = 0; c < contours->count; c++) {
        totalVCount += contours->contours[c].count;
        totalFCount += contours->contours[c].count - 2;
    }
    
    uint32* gindices = pushArray<uint32>(arena, 3*totalFCount);
    Vec3* gvertices = contours->contours[0].vertices;
    uint32* indices = gindices;
    
    uint32 cVCount = 0;
    uint32 cFCount = 0;
    
    for(uint32 c = 0; c < contours->count; c++) {
        Mesh3D* mesh = &meshes[c];
        memset(mesh, 0, sizeof(Mesh3D));
        
        Contour* contour = &contours->contours[c];
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
        
        for(uint32 f = 0; f < fCount; f++) {
            
            real32 minEdgeLength = 1000000;
            uint32 mi0, mi1, mi2;
            uint32 l = 0;
            
            for(uint32 i = 0; i < vCount-f; i++) {
                uint32 i0 = nextFree(excluded, vCount, l);
                uint32 i1 = nextFree(excluded, vCount, (i0+1)%vCount);
                uint32 i2 = nextFree(excluded, vCount, (i1+1)%vCount);
                
                if(isCCW(verts[i0], verts[i1], verts[i2])) {
                    real32 eLength = length(verts[i2]-verts[i0]);
                    if(eLength < minEdgeLength) {
                        minEdgeLength = eLength;
                        mi0 = i0;
                        mi1 = i1;
                        mi2 = i2;
                    }
                }
                l = i1;
            }
            excluded[mi1] = true;
            indices[3*f] = mi0;
            indices[3*f+1] = mi1;
            indices[3*f+2] = mi2;
        }
        indices += 2*3*fCount;
        popArray<bool>(arena, vCount);
    }
}

//void triangulateContours(MemoryArena* arena, Contours* contours, Mesh3D* meshes) {
//    
//    uint32 totalVCount = 0;
//    uint32 totalFCount = 0;
//    for(uint32 c = 0; c < contours->count; c++) {
//        totalVCount += contours->contours[c].count;
//        totalFCount += contours->contours[c].count - 2;
//    }
//    
//    uint32* gindices = pushArray<uint32>(arena, 3*totalFCount);
//    Vec3* gvertices = contours->contours[0].vertices;
//    uint32* indices = gindices;
//    
//    uint32 iOffset = 0;
//    
//    uint32 cVCount = 0;
//    uint32 cFCount = 0;
//    
//    for(uint32 c = 0; c < 3; c++) {
//        Contour* contour = &contours->contours[c];
//        Vec3* verts = contour->vertices;
//        ASSERT(verts == gvertices + cVCount);
//        
//        uint32 vCount = contour->count;
//        uint32 fCount = vCount - 2;
//        
//        cVCount += vCount;
//        cFCount += fCount;
//        
//        bool* excluded = pushArrayZeroed<bool>(arena, vCount);
//        
//        for(uint32 f = 0; f < fCount; f++) {
//            
//            real32 minEdgeLength = 1000000;
//            uint32 mi0, mi1, mi2;
//            uint32 l = 0;
//            
//            for(uint32 i = 0; i < vCount-f; i++) {
//                uint32 i0 = nextFree(excluded, vCount, l);
//                uint32 i1 = nextFree(excluded, vCount, i0+1);
//                uint32 i2 = nextFree(excluded, vCount, i1+1);
//                
//                if(isCCW(verts[i0], verts[i1], verts[i2])) {
//                    real32 eLength = length(verts[i2]-verts[i0]);
//                    if(eLength < minEdgeLength) {
//                        minEdgeLength = eLength;
//                        mi0 = i0;
//                        mi1 = i1;
//                        mi2 = i2;
//                    }
//                }
//                l = i1;
//            }
//            excluded[mi1] = true;
//            indices[3*f] = mi0 + iOffset;
//            indices[3*f+1] = mi1 + iOffset;
//            indices[3*f+2] = mi2 + iOffset;
//        }
//        iOffset += vCount;
//        indices += 3*fCount;
//        popArray<bool>(arena, vCount);
//    }
//    
//    memset(mesh, 0, sizeof(Mesh3D));
//    mesh->positions = gvertices;
//    mesh->indices = gindices;
//    mesh->vCount = cVCount;
//    mesh->fCount = cFCount;
//}
