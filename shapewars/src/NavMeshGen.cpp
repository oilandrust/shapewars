//
//  NavMeshGen.cpp
//  GetTheFlag
//
//  Created by Olivier on 05/12/15.
//
//

#define SMOOTH_DISTANCE_FIELD 1
#define MERGE_SMALL_REGIONS 1

#include "NavMeshGen.h"
#include <algorithm>

void initializeNavMesh(Memory* memory, Debug* debug, Level* level, NavMesh* navMesh, uint32 fieldWidth, uint32 fieldHeight, real32 radius)
{
    // Create binary raster of the level at the requested resolution.
    // 0 -> not walkable
    // 1 -> walkable
    uint8* rasterData = pushArray(&memory->temporaryArena, fieldHeight * fieldHeight, (uint8)0);
	LevelRaster raster = { rasterData, fieldWidth, fieldHeight };
    
	real32 cellWidth = (real32)level->width / (real32)fieldWidth;
	real32 cellHeight = (real32)level->height / (real32)fieldHeight;

    for (uint32 j = 0; j < fieldHeight; j++) {
        uint32 jw = (uint32)(level->height * (real32)j / (real32)fieldHeight);    
        for (uint32 i = 0; i < fieldWidth; i++) {
            uint32 iw = (uint32)(level->width * (real32)i / (real32)fieldWidth);
            if (level->tiles[iw + jw * level->width]) {
                rasterData[i + j * fieldHeight] = 1;
            }
        }
    }

    // Distance field.
    DistanceField distanceField;
    genDistanceField(&memory->temporaryArena, &raster, &distanceField);

    debug->distanceFieldTexId = debugDistanceFieldCreateTexture(&memory->temporaryArena, &distanceField);

    // Region Ids.
    RegionIdMap regionIds;
	memset(&regionIds, 0, sizeof(regionIds));
	regionIds.cellWidth = cellWidth;
	regionIds.cellHeight = cellHeight;

	real32 fieldRadius = radius * (real32) fieldWidth / level->width;
    genRegions(&memory->temporaryArena, &distanceField, &regionIds, fieldRadius);

    debug->idsTexId = debugRegionsCreateTexture(&memory->temporaryArena, &regionIds);

    // Contours
    ContourSet contours;
	memset(&contours, 0, sizeof(ContourSet));
    genContours(&memory->temporaryArena, &regionIds, &contours);

    debug->contourCount = contours.count;
    debug->contourVaos = pushArray<GLuint>(&memory->temporaryArena, contours.count);
    debug->contourICounts = pushArray<uint32>(&memory->temporaryArena, contours.count);
    for (uint32 i = 0; i < contours.count; i++) {
        uint32 vCount = contours.contours[i].count;
        Vec3* vertices = contours.contours[i].vertices;
        debug->contourVaos[i] = createVertexArray(vertices, vCount);
        debug->contourICounts[i] = vCount;
    }

	// Triangulated contours.
    Mesh3D* triangulatedCountours = pushArray<Mesh3D>(&memory->temporaryArena, contours.count);
    triangulateContours(&memory->temporaryArena, &contours, triangulatedCountours);

    debug->contourMeshCount = contours.count;
    debug->contourMeshes = pushArray<GLuint>(&memory->temporaryArena, contours.count);
    debug->contourMeshesIndices = pushArray<uint32>(&memory->temporaryArena, contours.count);
    for (uint32 i = 0; i < contours.count; i++) {
        debug->contourMeshes[i] = createIndexedVertexArray(&triangulatedCountours[i]);
        debug->contourMeshesIndices[i] = 3 * triangulatedCountours[i].fCount;
    }

    // NavMesh and connectivity.
    DualMesh dual;
    buildNavMesh(memory, &contours, triangulatedCountours, navMesh, &dual);

    debug->navPolyCount = navMesh->polyCount;
    debug->polyVaos = pushArray<GLuint>(&memory->temporaryArena, navMesh->polyCount);
    debug->polyICounts = pushArray<uint32>(&memory->temporaryArena, navMesh->polyCount);
    for (uint32 i = 0; i < navMesh->polyCount; i++) {
        debug->polyVaos[i] = createIndexedVertexArray(navMesh->vertices, navMesh->vertCount,
            navMesh->polygons + 2 * i * navMesh->maxVertPerPoly, navMesh->maxVertPerPoly);
        debug->polyICounts[i] = polyVertCount(navMesh, i);
    }
    debug->dualVao = createIndexedVertexArray(dual.vertices, dual.vertCount, dual.indices, dual.indCount);
    debug->dualICount = dual.indCount;
}

static inline uint32 sep_euclid(uint32* g, uint32 w, int32 j, int32 i, int32 u) {
	ASSERT(i < u);
	uint32 gu = g[u + j*w];
	uint32 gi = g[i + j*w];
	return (u*u - i*i + gu*gu - gi*gi) / (2*(u-i));
}

static inline int32 sep_manhatan(uint32* g, uint32 w, int32 j, int32 i, int32 u) {
	ASSERT(i < u);
	int32 gu = g[u + j*w];
	int32 gi = g[i + j*w];
	if (gu >= gu + u - i) {
		return w + w;
	}
	if (gi >= gu + u - i) {
		return -2 * w;
	}
	return (gu - gi + i + u) / 2;
}

static inline uint32 f_euclid(uint32* g, uint32 w, uint32 j, uint32 x, uint32 i) {
	uint32 d = x > i? x - i: i - x;
	uint32 gv = g[i+j*w];
	return d*d + gv*gv;
}

static inline uint32 f_manhatan(uint32* g, uint32 w, uint32 j, uint32 x, uint32 i) {
	uint32 d = x > i ? x - i : i - x;
	uint32 gv = g[i + j*w];
	return d + gv;
}

void genDistanceField(MemoryArena* arena, LevelRaster* level, DistanceField* field)
{
    field->width = level->width;
    field->height = level->height;
    uint8* levelMap = level->raster;

    uint32 w = field->width;
    uint32 h = field->height;
    size_t count = w * h;

    real32* distanceField = pushArray<real32>(arena, count);
	uint32* g = pushArray<uint32>(arena, count);

    // Compute the distance field.
	// Algorithm from: http://fab.cba.mit.edu/classes/S62.12/docs/Meijster_distance.pdf
	// First phase.
	// For each column
	for (uint32 i = 0; i < w; i++) {
		// top to bottom.
		if (levelMap[i] != 0) {
			g[i] = 0;
		}
		else {
			g[i] = w+h;
		}
		for (uint32 j = 1; j < h; j++) {
			if (levelMap[i + j*w] != 0) {
				g[i + j*w] = 0;
			}
			else {
				g[i + j*w] = g[i+(j-1)*w] + 1;
			}
		}

		// bottom to top.
		for (int32 j = h - 2; j >= 0; j--) {
			if (g[i+(j+1)*w] < g[i+j*w]) {
				g[i + j*w] = g[i + (j + 1)*w] + 1;
			}
		}
	}

	int32* s = pushArray<int32>(arena, w);
	int32* t = pushArray<int32>(arena, w);
	for (uint32 j = 0; j < h; j++) {
		int32 q = 0;
		s[0] = 0;
		t[0] = 0;
		for (uint32 u = 1; u < w; u++) {	
			while (q >= 0 && f_euclid(g, w, j, t[q], s[q]) > f_euclid(g, w, j, t[q], u)) {
				q--;
			}
			if (q < 0) {
				q = 0;
				s[0] = u;
			}
			else {
				uint32 ww = 1 + sep_euclid(g, w, j, s[q], u);
				if (ww < w) {
					q++;
					s[q] = u;
					t[q] = ww;
				}
			}
		}

		for (int32 u = w - 1; u >= 0; u--) {
			uint32 sq = s[q];
			distanceField[u + w*j] = -sqrtf((real32)f_euclid(g, w, j, u, sq));
			if (u == t[q]) q--;
		}
	}
	popArray<uint32>(arena, w);
	popArray<uint32>(arena, w);
	popArray<uint32>(arena, count);
	
	real32 minVal = 0;
	for (uint32 i = 0; i < count; i++) {
		if (distanceField[i] < minVal) {
			minVal = distanceField[i];
		}
	}
    
    field->minVal = minVal;
	field->field = distanceField;
   
#if SMOOTH_DISTANCE_FIELD
	real32* avgField = pushArray<real32>(arena, w * h);

	// smooth
    for (uint32 j = 0; j < h; j++) {
        for (uint32 i = 0; i < w; i++) {
            uint32 c = 1;
            real32 avg = distanceField[i + j * w];
            if (avg == 0) {
                avgField[i + j * w] = 0;
                continue;
            }
            // top
            if (j + 1 < h) {
                avg += distanceField[i + (j + 1) * w];
                c++;
            }
            // top right
            if (j + 1 < h && i + 1 < w) {
                avg += distanceField[i + 1 + (j + 1) * w];
                c++;
            }
            // right
            if (i + 1 < w) {
                avg += distanceField[i + 1 + j * w];
                c++;
            }
            // right bottom
            if (i + 1 < w && j > 0) {
                avg += distanceField[i + 1 + (j - 1) * w];
                c++;
            }
            // bottom
            if (j > 0) {
                avg += distanceField[i + (j - 1) * w];
                c++;
            }
            // bottom left
            if (i > 0 && j > 0) {
                avg += distanceField[i - 1 + (j - 1) * w];
                c++;
            }
            // left
            if (i > 0) {
                avg += distanceField[i - 1 + j * w];
                c++;
            }
            // left top
            if (i > 0 && j + 1 < h) {
                avg += distanceField[i - 1 + (j + 1) * w];
                c++;
            }

            avgField[i + j * w] = avg / c;
        }
    }
	field->field = avgField;
#endif
}

GLuint debugDistanceFieldCreateTexture(MemoryArena* arena, DistanceField* field)
{
    uint32 w = field->width;
    uint32 h = field->height;
    real32 maxDist = field->minVal;
    real32* distanceField = field->field;

    uint8* distanceFieldTexData = pushArray<uint8>(arena, w * h);

    for (uint32 j = 0; j < h; j++) {
        for (uint32 i = 0; i < w; i++) {
            distanceFieldTexData[i + j * w] = (uint8)(255 * distanceField[i + j * w] / maxDist);
        }
    }
    Texture tex;
    tex.width = w;
    tex.height = h;
    tex.data = distanceFieldTexData;
    createTexture(&tex, GL_RED);

    popArray<uint8>(arena, w * h);

    return tex.texId;
}

struct IdMap {
	int32 neighboors[4];
	int32 counts[4];
	int32 ids;
};

static void increment(IdMap* map, int32 id) {
	for (int32 i = 0; i < map->ids; i++) {
		if (map->neighboors[i] == id) {
			map->counts[i]++;
			return;
		}
	}
	map->neighboors[map->ids] = id;
	map->counts[map->ids] = 1;
	map->ids++;
}

static int32 maximum(IdMap* map) {
	int32 maxI = -1;
	int32 maxC = -1;
	for (int32 i = 0; i < map->ids; i++) {
		if (map->counts[i] >= maxC) {
			maxC = map->counts[i];
			maxI = i;
		}
	}
	ASSERT(maxI >= 0);
	return map->neighboors[maxI];
}

static int32 mostImportantNeighbor(int32* regionIds_t0, uint32 width, uint32 height, uint32 i, uint32 j) {
	IdMap map;
	memset(&map, 0, sizeof(IdMap));
	// top
	if (j + 1 < height && regionIds_t0[i + (j + 1) * width] != -1)
		increment(&map, regionIds_t0[i + (j + 1) * width]);
	// top right
	if (j + 1 < height && i + 1 < width && regionIds_t0[i + 1 + (j + 1) * width] != -1)
		increment(&map, regionIds_t0[i + 1 + (j + 1) * width]);
	// right
	if (i + 1 < width && regionIds_t0[i + 1 + j * width] != -1)
		increment(&map, regionIds_t0[i + 1 + j * width]);
	// right bottom
	if (i + 1 < width && j > 0 && regionIds_t0[i + 1 + (j-1) * width] != -1)
		increment(&map, regionIds_t0[i + 1 + (j-1) * width]);
	// bottom
	if (j > 0 && regionIds_t0[i + (j - 1) * width] != -1)
		increment(&map, regionIds_t0[i + (j - 1) * width]);
	// bottom left
	if (j > 0 && i > 0 && regionIds_t0[i - 1 + (j - 1) * width] != -1)
		increment(&map, regionIds_t0[i - 1 + (j - 1) * width]);
	// left
	if (i > 0 && regionIds_t0[i - 1 + j * width] != -1)
		increment(&map, regionIds_t0[i - 1 + j * width]);
	// left top
	if (i > 0 && j + 1 < height && regionIds_t0[i - 1 + (j+1) * width] != -1)
		increment(&map, regionIds_t0[i - 1 + (j+1) * width]);

	return maximum(&map);
};

static void flood(real32* field, real32 level, int32* regionIds_t0, int32* regionIds_t1,
    uint32 width, uint32 height,
    uint32 i, uint32 j, int32 newId)
{
    // top
    if (j + 1 < height) {
        uint32 n = i + (j + 1) * width;
        if (field[n] < level && regionIds_t0[n] == -1) {
			int32 id = mostImportantNeighbor(regionIds_t0, width, height, i, j + 1);
			regionIds_t1[n] = id;
            regionIds_t0[n] = id;
            flood(field, level, regionIds_t0, regionIds_t1, width, height, i, j + 1, id);
        }
    }
    // right
    if (i + 1 < width) {
        uint32 n = i + 1 + j * width;
        if (field[n] < level && regionIds_t0[n] == -1) {
			int32 id = mostImportantNeighbor(regionIds_t0, width, height, i + 1, j);
			regionIds_t1[n] = id;
            regionIds_t0[n] = id;
            flood(field, level, regionIds_t0, regionIds_t1, width, height, i + 1, j, id);
        }
    }
    // bottom
    if (j > 0) {
        uint32 n = i + (j - 1) * width;
        if (field[n] < level && regionIds_t0[n] == -1) {
			int32 id = mostImportantNeighbor(regionIds_t0, width, height, i, j - 1);
			regionIds_t1[n] = id;
            regionIds_t0[n] = id;
            flood(field, level, regionIds_t0, regionIds_t1, width, height, i, j - 1, id);
        }
    }
    // left
    if (i > 0) {
        uint32 n = i - 1 + j * width;
        if (field[n] < level && regionIds_t0[n] == -1) {
			int32 id = mostImportantNeighbor(regionIds_t0, width, height, i - 1, j);
			regionIds_t1[n] = id;
            regionIds_t0[n] = id;
            flood(field, level, regionIds_t0, regionIds_t1, width, height, i - 1, j, id);
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

static void mergeIfIsolated(int32* regionIds_t0, uint32 width, uint32 height, uint32 i, uint32 j, int32 gid) {
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
        regionIds_t0[i + j * width] = mostImportantNeighbor(regionIds_t0, width, height, i, j);
        mergeIfIsolated(regionIds_t0, width, height, i, j + 1, gid);
        mergeIfIsolated(regionIds_t0, width, height, i + 1, j, gid);
        mergeIfIsolated(regionIds_t0, width, height, i, j - 1, gid);
        mergeIfIsolated(regionIds_t0, width, height, i - 1, j, gid);
    }
};

void genRegions(MemoryArena* arena, DistanceField* distanceField, RegionIdMap* regions, real32 radius)
{
    uint32 width = distanceField->width;
    uint32 height = distanceField->height;
    size_t count = width * height;

    int32* regionIds_t0 = pushArray<int32>(arena, count);
    int32* regionIds_t1 = pushArray<int32>(arena, count);

    regions->width = width;
    regions->height = height;
   
    real32 levelValue = distanceField->minVal;
    int32 nextId = 0;

    for (uint32 j = 0; j < height; j++) {
        for (uint32 i = 0; i < width; i++) {
            regionIds_t0[i + j * width] = -1;
        }
    }

    real32* sortedDistances = pushArray<real32>(arena, count);
	for (uint32 j = 0; j < height; j++) {
		for (uint32 i = 0; i < width; i++) {
			sortedDistances[i + j*width] = distanceField->field[i + j*width];
		}
	}
    std::sort(sortedDistances, sortedDistances + count);
    const float eps = 0.000001f;
    uint32 levelIndex = 0;
    levelValue = sortedDistances[0];

    while (levelValue < -radius) {
        for (uint32 j = 0; j < count; j++) {
            regionIds_t1[j] = -1;
        }

        float val = levelValue + eps;

        // Flood the existing regions.
        for (uint32 j = 0; j < height; j++) {
            for (uint32 i = 0; i < width; i++) {
                uint32 v = i + j * width;
                if (distanceField->field[v] < val) {
                    if (regionIds_t0[v] != -1) {
                        regionIds_t1[v] = regionIds_t0[v];
                        // flood neighboring pixels
                        flood(distanceField->field, val, regionIds_t0, regionIds_t1, width, height,
                            i, j, regionIds_t0[v]);
                    }
                }
            }
        }

        // Flood newly created regions.
        for (uint32 j = 0; j < height; j++) {
            for (uint32 i = 0; i < width; i++) {
                uint32 v = i + j * width;
                if (distanceField->field[v] < val) {
                    if (regionIds_t0[v] == -1 && regionIds_t1[v] == -1) {
                        // Create a new region.
                        regionIds_t1[v] = nextId++;
                        // flood neighboring pixels
                        floodNew(distanceField->field, val, regionIds_t0, regionIds_t1, width, height,
                            i, j, regionIds_t1[v]);
                    }
                }
            }
        }

        std::swap(regionIds_t0, regionIds_t1);
        while (levelIndex < count && sortedDistances[levelIndex] == levelValue) {
            levelIndex++;
        }
        levelValue = sortedDistances[levelIndex];
    }
    // sorted distances.
    popArray<real32>(arena, count);
	regions->ids = regionIds_t0;
    regions->regionCount = nextId;

#if MERGE_SMALL_REGIONS
	regionIds_t0 = regions->ids;
    // We don't want cells with only one neighboor of the same id.
    for (uint32 j = 0; j < height; j++) {
        for (uint32 i = 0; i < width; i++) {
			int32 id = regionIds_t0[i + j*width];
            if (id != -1) {
                mergeIfIsolated(regionIds_t0, width, height, i, j, id);
            }
			ASSERT(id == -1 || (id >= 0 && id < nextId));
        }
    }
#endif

    regions->lastId = nextId;

    // TODO:
    // If a regions wrap around a non walkable region, these non walkable region will become walkable.
    // So we go through unwalkable regions, and if they have only one region as neighboor, split this region.

    // regionIds_t1
    //popArray<int32>(arena, count);
}

GLuint debugRegionsCreateTexture(MemoryArena* arena, RegionIdMap* idMap)
{
    uint32 idCount = idMap->lastId;
    uint32 w = idMap->width;
    uint32 h = idMap->height;
    int32* ids = idMap->ids;

    // Write a segment texture with id->color map.
    RGBA* segmentsTexData = pushArray<RGBA>(arena, w * h);
    for (uint32 j = 0; j < h; j++) {
        for (uint32 i = 0; i < w; i++) {
            uint32 index = i + j * w;
            if (ids[index] != -1) {
                int32 cId = ids[index];
                RGB col = randomColor(&g_debugDraw, cId);
				RGBA color = { col.r, col.g, col.b, 255 };
                segmentsTexData[index] = color;
            }
            else {
				RGBA black = { 0, 0, 0, 0 };
                segmentsTexData[index] = black;
            }
        }
    }

    Texture tex;
    tex.width = w;
    tex.height = h;
    tex.data = segmentsTexData;
    createTexture(&tex, GL_RGBA);

    popArray<RGBA>(arena, w * h);
   
    return tex.texId;
}

static int32 idAt(int32* ids, uint32 w, uint32 i, uint32 j)
{
	if (i >= w || j >= w) {
		return -1;
	}
	return ids[i + j * w];
};

enum Dir {
	DIR_UP = 0,
	DIR_RIGHT,
	DIR_DOWN,
	DIR_LEFT
};

static void moveForward(Dir fDir, uint32& x, uint32& y) {
	switch (fDir) {
	case DIR_UP:
		y--;
		break;
	case DIR_RIGHT:
		x++;
		break;
	case DIR_DOWN:
		y++;
		break;
	case DIR_LEFT:
		x--;
		break;
	default:
		break;
	}
}

static int32 idAtRight(int32* ids, uint32 w, uint32 h, uint32 i, uint32 j, Dir dir)
{
    switch (dir) {
    case DIR_UP:
        return j == 0 ? -1 : idAt(ids, w, i, j - 1);
    case DIR_RIGHT:
        return i + 1 >= w ? -1 : idAt(ids, w, i + 1, j);
    case DIR_DOWN:
        return j + 1 >= h ? -1 : idAt(ids, w, i, j + 1);
    case DIR_LEFT:
        return i == 0 ? -1 : idAt(ids, w, i - 1, j);
    }
    return -1;
};

static bool isEdge(int32* ids, uint32 w, uint32 h, uint32 i, uint32 j, Dir dir)
{
    switch (dir) {
    case DIR_UP:
        return j == 0 || idAt(ids, w, i, j) != idAt(ids, w, i, j - 1);
    case DIR_RIGHT:
        return i + 1 >= w || idAt(ids, w, i, j) != idAt(ids, w, i + 1, j);
    case DIR_DOWN:
        return j + 1 >= h || idAt(ids, w, i, j) != idAt(ids, w, i, j + 1);
    case DIR_LEFT:
        return i == 0 || idAt(ids, w, i, j) != idAt(ids, w, i - 1, j);
    }
    return false;
};

static Vec3 vertexAt(uint32 x, uint32 y, Dir fDir, real32 mw, real32 mh, real32 cw, real32 ch) {
	Vec3 cellCenter = Vec3((x + .5f) * cw, mh - (y - .5f) * ch - ch, .5f);
	Vec3 offset;
	
	switch (fDir) {
	case DIR_UP:
		offset = Vec3(.5f * cw, -.5f * ch, .0f);
		break;
	case DIR_RIGHT:
		offset = Vec3(-.5f * cw, -.5f * ch, .0f);
		break;
	case DIR_DOWN:
		offset = Vec3(-.5f * cw, .5f * ch, .0f);
		break;
	case DIR_LEFT:
		offset = Vec3(.5f * cw, .5f * ch, .0f);
		break;
	}

	return cellCenter + offset;
}

bool isVertexAt(uint32 x, uint32 y, Dir rDir, int32* ids, uint32 w, uint32 h) {
	IdMap map;
	memset(&map, 0, sizeof(IdMap));
	int32 id = ids[x + y * w];
	int32 idRight = idAtRight(ids, w, h, x, y, rDir);
	Dir bDir = (Dir)((rDir + 1) % 4);
	int32 idBack = idAtRight(ids, w, h, x, y, bDir);
	moveForward(rDir, x, y);
	int32 idBackRight = idAtRight(ids, w, h, x, y, bDir);

	if (idRight == -1 || idBack == -1 || idBackRight == -1) {
		return true;
	}

	increment(&map, id);
	increment(&map, idRight);
	increment(&map, idBack);
	increment(&map, idBackRight);

	return map.ids > 2;
}

void walkCountour(MemoryArena* arena, Contour* contour, int32* ids, uint32 w, uint32 h, real32 mw, real32 mh, real32 cw, real32 ch, uint32 i, uint32 j)
{
	uint32 count = 0;
	Vec3* verts = pushArray<Vec3>(arena, w * h);
	
	// We visit in order from top-left to bottom so the direction must be DOWN for ccw order.
	Dir fDir = DIR_DOWN;
	Dir rDir = (Dir)((fDir + 1) % 4);

	uint32 x = i;
	uint32 y = j;
	uint32 xb = i;
	uint32 yb = j;

	int32 idRight = idAtRight(ids, w, h, x, y, rDir);
	do {
		bool edgeForward = isEdge(ids, w, h, x, y, fDir);
		bool edgeRight = isEdge(ids, w, h, x, y, rDir);
		
		// No wall in front, wall still on our right.
		if (edgeRight && !edgeForward) {
			moveForward(fDir, x, y);
	
			int32 newIdRight = idAtRight(ids, w, h, x, y, rDir);
			bool edgeRight = isEdge(ids, w, h, x, y, rDir);

			if (idRight != newIdRight && edgeRight) {
				if (isVertexAt(x, y, rDir, ids, w, h)) {
					verts[count++] = vertexAt(x, y, fDir, mw, mh, cw, ch);
				}
			}
			idRight = newIdRight;
			continue;
		}

		// Wall in front and on the right.
		if (edgeRight && edgeForward) {
			// turn left
			rDir = fDir;
			fDir = fDir == 0 ? DIR_LEFT : (Dir)((fDir - 1) % 4);

			if (isVertexAt(x, y, rDir, ids, w, h)) {
				verts[count++] = vertexAt(x, y, fDir, mw, mh, cw, ch);
			}
			
			idRight = idAtRight(ids, w, h, x, y, rDir);
			continue;
		}

		// No wall on the right.
		if (!edgeRight) {
			// turn right.
			fDir = rDir;
			rDir = (Dir)((fDir + 1) % 4);
			moveForward(fDir, x, y);

			if (isVertexAt(x, y, rDir, ids, w, h)) {
				verts[count++] = vertexAt(x, y, fDir, mw, mh, cw, ch);
			}
			
			idRight = idAtRight(ids, w, h, x, y, rDir);
			continue;
		}
		ASSERT(false);	
	} while (x != xb || y != yb || fDir != DIR_DOWN);

	// resize verts
	popArray<Vec3>(arena, w * h - count);

	contour->count = count;
	contour->vertices = verts;
}


static void simplifyContour(RegionIdMap* regions, Contour* contour)
{

}

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

	real32 widthInMeters = regions->width * regions->cellWidth;
	real32 heightInMeters = regions->height * regions->cellHeight;

    // Visit all cell to find beginings of contours, mark visited contour.
    for (uint32 j = 0; j < h; j++) {
        for (uint32 i = 0; i < w; i++) {

            int32 cellId = ids[i + j * w];
            if (cellId == -1 || visited[cellId]) {
                continue;
            }
            // Check if this is a border cell to start a region
            if (!isEdge(ids, w, h, i, j, DIR_UP)
                && !isEdge(ids, w, h, i, j, DIR_RIGHT)
                && !isEdge(ids, w, h, i, j, DIR_DOWN)
                && !isEdge(ids, w, h, i, j, DIR_LEFT)) {
                continue;
            }
            visited[cellId] = 1;

            Contour* contour = &contours->contours[contours->count++];
			walkCountour(arena, contour, ids, w, h, widthInMeters, heightInMeters, regions->cellWidth, regions->cellHeight, i, j);
        }
    }

	for (uint32 c = 0; c < contours->count; c++) {
		Contour* contour = &contours->contours[c];
		simplifyContour(regions, contour);
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

static bool segmentsIntersect(const Vec2& p0, const Vec2& p1, const Vec2& p2, const Vec2& p3) {
	real32 s1_x, s1_y, s2_x, s2_y;
	s1_x = p1.x - p0.x;     s1_y = p1.y - p0.y;
	s2_x = p3.x - p2.x;     s2_y = p3.y - p2.y;

	real32 d0 = (-s2_x * s1_y + s1_x * s2_y);
	real32 d1 = (-s2_x * s1_y + s1_x * s2_y);

	if (d0 < 0.000001 || d1 < 0.000001) {
		return false;
	}

	real32 s, t;
	s = (-s1_y * (p0.x - p2.x) + s1_x * (p0.y - p2.y)) / d0;
	t = (s2_x * (p0.y - p2.y) - s2_y * (p0.x - p2.x)) / d1;

	if (s > 0 && s < 1 && t > 0 && t < 1) {
		return true;
	}

	return false;
}

static bool intersectsContour(Vec3* verts, uint32 count, const Vec3& p0, const Vec3& p1) {
	for (uint32 s = 0; s < count; s++) {
		Vec3 s0 = verts[s];
		Vec3 s1 = verts[(s + 1) % count];

		if (segmentsIntersect(s0.xy(), s1.xy(), p0.xy(), p1.xy())) {
			return true;
		}
	}

	return false;
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
        Contour* contour = contours->contours + c;
        Vec3* verts = contour->vertices;
        ASSERT(verts == gvertices + cVCount);

        uint32 vCount = contour->count;
        uint32 fCount = vCount - 2;

        Mesh3D* mesh = meshes + c;
        memset(mesh, 0, sizeof(Mesh3D));
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

                if (isCCW(verts[i0], verts[i1], verts[i2]) && !intersectsContour(verts, vCount, verts[i2], verts[i0])) {
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
    Vec3 p(0, 0, 0);
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

void buildNavMesh(Memory* memory, ContourSet* contours, Mesh3D* triMeshes, NavMesh* mesh, DualMesh* dual)
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
    Vec3* meshVerts = pushArray<Vec3>(&memory->persistentArena, totalVCount);
    // Faces and connectivity for the final mesh
    uint32* meshPolys = pushArray<uint32>(&memory->persistentArena, 2 * maxVertsPerPoly * totalTriCount, NULL_INDEX);

    mesh->vertCount = 0;
    mesh->vertices = meshVerts;
    mesh->polyCount = 0;
    mesh->polygons = meshPolys;

    for (uint32 c = 0; c < contours->count; c++) {
        Mesh3D* triMesh = triMeshes + c;
        Vec3* triVerts = triMesh->positions;
        uint32* triInds = triMesh->indices;

        // Index mapping because we are merging vertices.
        uint32* iMap = pushArray<uint32>(&memory->temporaryArena, triMesh->vCount);
        for (uint32 v = 0; v < triMesh->vCount; v++) {
            iMap[v] = findOrAddVertex(mesh, triVerts[v]);
        }

        // Temporary polygon soup intialized with the triangles.
        uint32* polys = pushArray<uint32>(&memory->temporaryArena, maxVertsPerPoly * triMesh->fCount);
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
                mergePolygons(&memory->temporaryArena, polys, nPolys, maxVertsPerPoly, p0, p1, v0, v1);

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

        popArray<uint32>(&memory->temporaryArena, maxVertsPerPoly * triMesh->fCount);
        popArray<uint32>(&memory->temporaryArena, triMeshes->vCount);
    }

    computePolygonAdjacency(&memory->temporaryArena, mesh, dual);
}

bool checkNavMesh(NavMesh* mesh)
{
    for (uint32 i = 0; i < mesh->polyCount; i++) {
        ASSERT(polyVertCount(mesh, i) >= 3);
    }
    return true;
}
