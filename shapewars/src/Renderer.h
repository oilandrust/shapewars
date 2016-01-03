#ifndef RENDERERH
#define RENDERERH

#include "Mat3.h"
#include "Mat4.h"
#include "OpenGLClient.h"
#include "ShapeWars.h"
#include "Vec2.h"

enum RenderPieceType {
    TRIMESH_COLOR,
    TRIMESH_TEXTURE,
    TRIMESH_WIREFRAME,
    ARRAY_POINTS,
    ARRAY_LINE_LOOP,
    ARRAY_LINE_STRIP,
    INDEXED_ARRAY_LINES,
    INDEXED_ARRAY_LINE_LOOP,
};

struct RenderPiece {
    RenderPieceType type;
    Mat3 rotation;
    Vec3 position;
    Vec3 size;
    Vec3 color;
    GLuint vao;
    uint32 iCount;
    GLuint texId;
    Shader* shader;
};

#define MAX_RENDER_PICES 1024

struct Renderer {
    RenderPiece* renderQueue;
    uint32 pieceCount;

    Shader flatDiffShader;
    Shader texDiffShader;
    Shader flatColorShader;
    Shader groundShader;
};

/**
 * Initialization.
 */
void intializeRenderer(MemoryArena* arena, Renderer* renderer);

void reloadShaders(Renderer* renderer);

void initializeShader(Shader* shader, const char* vsSourcePath, const char* fsSourcePath);

/**
 * Add render requests.
 */
void pushMeshPiece(Renderer* renderer, Shader* shader,
    GLuint vao, uint32 iCount,
    const Mat3& rot, const Vec3& size, const Vec3& pos, const Vec3& color);

void pushMeshPieceTextured(Renderer* renderer, Shader* shader,
    GLuint vao, uint32 iCount, GLuint texId,
    const Mat3& rot, const Vec3& size, const Vec3& pos);

void pushMeshPieceWireframe(Renderer* renderer, Shader* shader,
    GLuint vao, uint32 iCount,
    const Mat3& rot, const Vec3& size, const Vec3& pos, const Vec3& color);

void pushArrayPiece(Renderer* renderer, Shader* shader,
    GLuint vao, uint32 count, RenderPieceType type,
    const Mat3& rot, const Vec3& size, const Vec3& pos, const Vec3& color);

void pushIndexedArrayPiece(Renderer* renderer, Shader* shader,
    GLuint vao, uint32 count, RenderPieceType type,
    const Mat3& rot, const Vec3& size, const Vec3& pos, const Vec3& color);

/**
 * Rendering.
 */
void rendererBeginFrame(Renderer* renderer);

void renderAll(Renderer* renderer, const Mat4& projection, const Mat4& view);

#endif