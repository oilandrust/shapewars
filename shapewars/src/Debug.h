//
//  Debug.h
//  GetTheFlag
//
//  Created by Olivier on 27/12/15.
//
//

#ifndef Debug_h
#define Debug_h

#include "NavMeshQuery.h"
#include "OpenGLClient.h"

struct Renderer;
struct TextRenderer;
struct Font;
struct Input;

struct DebugOption {
	const char* text;
	Rect2 bbox;
	bool* active;
	real32* value;
	real32 increment;
	bool hovered;
};

#define MAX_DEBUG_OPTIONS 32

#define RND_COLS 32

struct DebugDraw {
	GLuint vbo;
	GLuint vao;
	uint32 count;
	RGB randomColors[RND_COLS];
};

extern DebugDraw g_debugDraw;

void initializeDebugDraw(DebugDraw* dd);

inline RGB randomColor(DebugDraw* dd, uint32 index) {
	return dd->randomColors[index%RND_COLS];
}

void debugDrawLineStrip(DebugDraw* dd, Vec3* verts, uint32 count);

struct Debug {
	Font* font;
	DebugOption options[MAX_DEBUG_OPTIONS];
	uint32 optionCount;

    bool showDistanceField;
    bool showRegions;
    real32 planeSize;
    GLuint distanceFieldTexId;
    GLuint idsTexId;

    bool showContours;
    uint32 contourCount;
    GLuint* contourVaos;
    uint32* contourICounts;

    bool showTriRegions;
    uint32 contourMeshCount;
    GLuint* contourMeshes;
    uint32* contourMeshesIndices;

    bool showNavMesh;
    uint32 navPolyCount;
    GLuint* polyVaos;
    uint32* polyICounts;

    bool showDual;
    GLuint dualVao;
    uint32 dualICount;

    NavMesh* navMesh;

    bool showPath;

	real32* agentRadius;

	bool showWorld;
	bool showWalls;

    bool showText;
    real32 fps;
};

extern Debug g_debug;

void initalizeDebug(Debug* debug);

void debugHandleInput(Debug* debug, Input* input);

void renderDebug(Debug* debug, Renderer* renderer, TextRenderer* tr);

#endif /* Debug_h */
