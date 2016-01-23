//
//  debug.cpp
//  GetTheFlag
//
//  Created by Olivier on 27/12/15.
//
//

#include "Input.h"
#include "Mat3.h"
#include "Renderer.h"
#include "Text.h"
#include "Vec3.h"
#include "debug.h"

#include <cstdio>
#include <stdio.h>

Debug g_debug;
DebugDraw g_debugDraw;

void initializeDebugDraw(DebugDraw* dd) 
{
	dd->vbo = createBufferObject((Vec3*)NULL, 128, GL_DYNAMIC_DRAW);
	dd->vao = createVertexArray(dd->vbo);
}

void debugDrawLineStrip(DebugDraw* dd, Vec3* verts, uint32 count)
{
	updateBufferObject(dd->vbo, verts, count);
	dd->count = count;
}

static void defineOption(Font* font, DebugOption* options, uint32& count, Vec2& pos, const char* text, bool* debugVar, bool defaultValue = false)
{
	DebugOption* option = options + count;
	option->active = debugVar;
	if (debugVar) {
		*option->active = defaultValue;
	}
	option->text = text;
	Vec2 size = textSize(font, text);
	option->bbox = { pos - Vec2(0.f, size.y), pos + Vec2(size.x, 0.f) };
	pos.y += font->size;
	count++;
}

static void defineRealOption(Font* font, DebugOption* options, uint32& count, Vec2& pos, const char* text, real32* debugVar, real32 increment)
{
	DebugOption* option = options + count;
	option->value = debugVar;
	option->text = text;
	option->increment = increment;
	Vec2 size = textSize(font, text);
	option->bbox = { pos - Vec2(0.f, size.y), pos + Vec2(size.x, 0.f) };
	pos.y += font->size;
	count++;
}

void initalizeDebug(Debug* debug) 
{
	DebugOption* options = debug->options;
	memset(options, 0, MAX_DEBUG_OPTIONS * sizeof(DebugOption));
	uint32 count = 0;
	Font* font = debug->font;

	debug->showText = false;

	Vec2 pos((real32)font->size);
	defineOption(font, options, count, pos, "NavMesh:", NULL);
	defineOption(font, options, count, pos, "    Show distance field.", &debug->showDistanceField);
	defineRealOption(font, options, count, pos, "    Increase radius.", debug->agentRadius, 0.1f);
	defineRealOption(font, options, count, pos, "    Decrease radius.", debug->agentRadius, -0.1f);
	defineOption(font, options, count, pos, "    Show regions.", &debug->showRegions);
	defineOption(font, options, count, pos, "    Show region contours.", &debug->showContours);
	defineOption(font, options, count, pos, "    Show triangulated contours.", &debug->showTriRegions);
	defineOption(font, options, count, pos, "    Show polygon navmesh.", &debug->showNavMesh);
	defineOption(font, options, count, pos, "    Show navmesh dual (connectivity).", &debug->showDual);
	defineOption(font, options, count, pos, "Path finding:", NULL);
	defineOption(font, options, count, pos, "    Show path, also shows path polygons if navmesh active.", &debug->showPath);
	defineOption(font, options, count, pos, "Rendering:", NULL);
	defineOption(font, options, count, pos, "    Render worls.", &debug->showWorld, true);
	defineOption(font, options, count, pos, "    Render walls.", &debug->showWalls, true);
	defineOption(font, options, count, pos, "Misc:", NULL);
	defineOption(font, options, count, pos, "    (h): Hide/show debug menu.", &debug->showText);
	defineOption(font, options, count, pos, "    (r): Reload all shaders.", NULL);
	defineOption(font, options, count, pos, "    (s) : toggle full - screen.", NULL);

	debug->optionCount = count;
}

void debugHandleInput(Debug* debug, Input* input)
{
    if (input->keyStates[DEBUG_SHOW_MENU].clicked) {
        debug->showText = !debug->showText;
    }

	if (!debug->showText) {
		return;
	}

	uint32 count = debug->optionCount;
	DebugOption* options = debug->options;
	Vec2 mousePos(input->mouseX, input->mouseY);
	for (uint32 i = 0; i < count; i++) {
		DebugOption* opt = options + i;
		if (insideRect(opt->bbox, mousePos)) {
			opt->hovered = true;
		
			if (opt->active && input->keyStates[MOUSE_LEFT].clicked) {
				*opt->active = !(*opt->active);
			}
			if (opt->value && input->keyStates[MOUSE_LEFT].clicked) {
				*opt->value += opt->increment;
			}
		}
		else {
			opt->hovered = false;
		}
	}
}

void renderDebug(Debug* debug, Renderer* renderer, TextRenderer* tr)
{
    Mat3 identity3;
    identity(identity3);
    Vec3 groundSize(debug->planeSize);
    Vec3 groundCenter(.5f * debug->planeSize, .5f * debug->planeSize, 0.f);

    if (debug->showDistanceField) {
        pushPlanePieceTextured(renderer, &renderer->texDiffShader,
            debug->distanceFieldTexId, identity3, groundSize, groundCenter);
    }
    else if (debug->showRegions) {
        pushPlanePieceTextured(renderer, &renderer->texDiffShader,
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

	if (debug->showPath) {
		pushArrayPiece(renderer, &renderer->flatColorShader, g_debugDraw.vao, g_debugDraw.count, ARRAY_LINE_STRIP, Vec3(1.f));
	}

	if (!debug->showText) {
		return;
	}

	Vec3 colActive = Vec3(1.f);
	Vec3 colInactive = Vec3(.7f);
	Vec3 colHovered = Vec3(1.f, 1.f, 0.f);

	beginText(tr, 32.f, 0.f);
	uint32 count = debug->optionCount;
	DebugOption* options = debug->options;
	for (uint32 i = 0; i < count; i++) {
		DebugOption* opt = options + i;
		
		Vec3 col = opt->hovered ? colHovered : colActive;

		if(opt->active != NULL) {
			col = opt->hovered ? colHovered
				: (*opt->active ? colActive : colInactive);
		}
		
		pushText(tr, opt->text, Vec2(opt->bbox.min.x, opt->bbox.max.y), col);
	}
}
