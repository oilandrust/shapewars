#include "Renderer.h"
#include "Text.h"

#include <cstdio>

#define FONT_ATLAS_SIZE 512

const int fontBufferSize = 1 << 20;

void initalizeTextRenderer(Memory* memory, TextRenderer* tr)
{
    /* Load the font and create the texture atlas. */
    Font* font = &tr->defaultFont;
    font->size = 40;

    FILE* fontFile = fopen("data/OpenSans-Regular.ttf", "rb");
    ASSERT(fontFile);

    uint8* fontBuffer = pushArray<uint8>(&memory->temporaryArena, fontBufferSize);
    uint8* fontBitmap = pushArray<uint8>(&memory->temporaryArena, FONT_ATLAS_SIZE * FONT_ATLAS_SIZE);

    fread(fontBuffer, 1, fontBufferSize, fontFile);
    int32 ret = stbtt_BakeFontBitmap(fontBuffer, 0, (real32)font->size,
        fontBitmap, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE,
        32, 96, font->glyphs);

    ASSERT(ret != -1);

    glGenTextures(1, &font->texId);
    glBindTexture(GL_TEXTURE_2D, font->texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, fontBitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    resetArena(&memory->temporaryArena);

    /* Allocate the quad buffer. */
    tr->quads = pushArray<Vec2>(&memory->persistentArena, 6 * MAX_CHARS);
    tr->uvs = pushArray<Vec2>(&memory->persistentArena, 6 * MAX_CHARS);
    tr->quadCount = 0;

    tr->qVbo = createBufferObject(tr->quads, 6 * MAX_CHARS, GL_DYNAMIC_DRAW);
    tr->uvVbo = createBufferObject(tr->uvs, 6 * MAX_CHARS, GL_DYNAMIC_DRAW);

    // Create the vao
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    bindAttribBuffer(tr->qVbo, POS_ATTRIB_LOC, 2);
    bindAttribBuffer(tr->uvVbo, UV_ATTRIB_LOC, 2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    tr->vao = vao;
}

void beginText(TextRenderer* tr, real32 x, real32 y)
{
    tr->quadCount = 0;
    tr->offsetX = x;
    tr->offsetY = y + tr->defaultFont.size;
    tr->alignX = x;
}

void pushText(TextRenderer* tr, const int8* text)
{
    Font* font = &tr->defaultFont;
    Vec2* quads = tr->quads;
    Vec2* uvs = tr->uvs;
    uint32 count = tr->quadCount;

    while (*text) {
        if (*text == '\n') {
            tr->offsetX = tr->alignX;
            tr->offsetY += font->size;
        }
        else {
            ASSERT(count + 1 < MAX_CHARS);
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(font->glyphs, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE,
                *text - 32, &tr->offsetX, &tr->offsetY, &q, 1);

            uint32 qBegin = 6 * count;

            // triangle 1
            uvs[qBegin] = Vec2(q.s0, q.t0);
            quads[qBegin] = Vec2(q.x0, q.y0);
            uvs[qBegin + 1] = Vec2(q.s1, q.t0);
            quads[qBegin + 1] = Vec2(q.x1, q.y0);
            uvs[qBegin + 2] = Vec2(q.s1, q.t1);
            quads[qBegin + 2] = Vec2(q.x1, q.y1);

            // triangle 2
            uvs[qBegin + 3] = Vec2(q.s0, q.t1);
            quads[qBegin + 3] = Vec2(q.x0, q.y1);
            uvs[qBegin + 4] = Vec2(q.s0, q.t0);
            quads[qBegin + 4] = Vec2(q.x0, q.y0);
            uvs[qBegin + 5] = Vec2(q.s1, q.t1);
            quads[qBegin + 5] = Vec2(q.x1, q.y1);

            count++;
        }
        text++;
    }

    tr->quadCount = count;
}

void pushLine(TextRenderer* tr, const int8* text)
{
    tr->offsetY += tr->defaultFont.size;
    tr->offsetX = tr->alignX;

    pushText(tr, text);
}

void renderText(TextRenderer* tr)
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    updateBufferObject(tr->qVbo, tr->quads, 6 * tr->quadCount);
    updateBufferObject(tr->uvVbo, tr->uvs, 6 * tr->quadCount);

    Shader* shader = tr->shader;
    glUseProgram(shader->progId);
    glUniform2f(shader->resolutionLoc, tr->screenRes.x, tr->screenRes.y);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(shader->diffTexLoc, 0);
    glBindTexture(GL_TEXTURE_2D, tr->defaultFont.texId);

    glBindVertexArray(tr->vao);
    glDrawArrays(GL_TRIANGLES, 0, 6 * tr->quadCount);

    tr->quadCount = 0;
}