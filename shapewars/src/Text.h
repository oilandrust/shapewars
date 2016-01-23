#ifndef TEXTH
#define TEXTH


#ifdef _WIN32
#include <windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
#else
#include <GL/glew.h>
#include <OpenGL/OpenGL.h>
#endif

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

Vec2 textSize(Font* font, const char* text);

struct TextRenderer {
    Font defaultFont;
    Shader* shader;

    real32 offsetY;
    real32 offsetX;
    real32 alignX;

    Vec2 screenRes;

    Vec2* quads;
    Vec2* uvs;
	Vec3* colors;
    uint32 quadCount;
    GLuint vao;

    // dynamic vertex buffers.
    GLuint qVbo;
    GLuint uvVbo;
	GLuint colorVbo;
};

void beginText(TextRenderer* textRenderer, real32 x, real32 y);

void initalizeTextRenderer(Memory* memory, TextRenderer* textRenderer);

void pushText(TextRenderer* textRenderer, const int8* text, const Vec2& position, const Vec3& color = Vec3(1.f, 1.f, 1.f));

void pushLine(TextRenderer* textRenderer, const int8* text, const Vec3& color = Vec3(1.f, 1.f, 1.f));

void renderText(TextRenderer* textRenderer);

#endif