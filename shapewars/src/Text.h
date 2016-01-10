#ifndef TEXTH
#define TEXTH

#include <GL/glew.h>
#include <OpenGL/OpenGL.h>
#include <stb_truetype.h>

#include "ShapeWars.h"
#include "Vec2.h"

#define MAX_CHARS 1024

struct Font {
    // ASCII 32..126 is 95 glyphs
    stbtt_bakedchar glyphs[96];
    GLuint texId;
    int32 size;
};

struct TextRenderer {
    Font defaultFont;
    Shader* shader;

    real32 offsetY;
    real32 offsetX;
    real32 alignX;

    Vec2 screenRes;

    Vec2* quads;
    Vec2* uvs;
    uint32 quadCount;
    GLuint vao;

    // dynamic vertex buffers.
    GLuint qVbo;
    GLuint uvVbo;
};

void beginText(TextRenderer* textRenderer, real32 x, real32 y);

void initalizeTextRenderer(Memory* memory, TextRenderer* textRenderer);

void pushText(TextRenderer* textRenderer, const int8* text);

void pushLine(TextRenderer* textRenderer, const int8* text);

void renderText(TextRenderer* textRenderer);

#endif