#include "Renderer.h"

void intializeRenderer(MemoryArena* arena, Renderer* renderer)
{
    renderer->renderQueue = pushArray<RenderPiece>(arena, MAX_RENDER_PICES);
    renderer->pieceCount = 0;

    reloadShaders(renderer);
    logOpenGLErrors();
}

void rendererBeginFrame(Renderer* /*renderer*/)
{
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);

    glPointSize(4.0f);

    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    // Set Global render states
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);

    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    logOpenGLErrors();

    glClear(GL_COLOR_BUFFER_BIT);
}

void reloadShaders(Renderer* renderer)
{
    // 3d textured diffuse shader
    if (renderer->texDiffShader.progId != 0) {
        glDeleteShader(renderer->texDiffShader.progId);
    }
    createShaderProgram(&renderer->texDiffShader, "shaders/texture_diffuse.vs", "shaders/texture_diffuse.fs");
    logOpenGLErrors();

    glBindAttribLocation(renderer->texDiffShader.progId, POS_ATTRIB_LOC, "position");
    glBindAttribLocation(renderer->texDiffShader.progId, NORM_ATTRIB_LOC, "normal");
    glBindAttribLocation(renderer->texDiffShader.progId, UV_ATTRIB_LOC, "uv");

    renderer->texDiffShader.projLoc = glGetUniformLocation(renderer->texDiffShader.progId, "projection");
    renderer->texDiffShader.viewLoc = glGetUniformLocation(renderer->texDiffShader.progId, "view");
    renderer->texDiffShader.diffTexLoc = glGetUniformLocation(renderer->texDiffShader.progId, "diffuse");

    renderer->texDiffShader.posLoc = glGetUniformLocation(renderer->texDiffShader.progId, "entity_position");
    renderer->texDiffShader.sizeLoc = glGetUniformLocation(renderer->texDiffShader.progId, "entity_size");
    renderer->texDiffShader.rotLoc = glGetUniformLocation(renderer->texDiffShader.progId, "entity_rotation");

    // 3d color diffuse shader
    if (renderer->flatDiffShader.progId != 0) {
        glDeleteShader(renderer->flatDiffShader.progId);
    }
    createShaderProgram(&renderer->flatDiffShader, "shaders/flat_diffuse.vs", "shaders/flat_diffuse.fs");
    logOpenGLErrors();

    glBindAttribLocation(renderer->flatDiffShader.progId, POS_ATTRIB_LOC, "position");
    glBindAttribLocation(renderer->flatDiffShader.progId, NORM_ATTRIB_LOC, "normal");

    renderer->flatDiffShader.projLoc = glGetUniformLocation(renderer->flatDiffShader.progId, "projection");
    renderer->flatDiffShader.viewLoc = glGetUniformLocation(renderer->flatDiffShader.progId, "view");

    renderer->flatDiffShader.posLoc = glGetUniformLocation(renderer->flatDiffShader.progId, "entity_position");
    renderer->flatDiffShader.sizeLoc = glGetUniformLocation(renderer->flatDiffShader.progId, "entity_size");
    renderer->flatDiffShader.rotLoc = glGetUniformLocation(renderer->flatDiffShader.progId, "entity_rotation");
    renderer->flatDiffShader.diffuseLoc = glGetUniformLocation(renderer->flatDiffShader.progId, "entity_color");

    // 3d color flat shader
    if (renderer->flatDiffShader.progId != 0) {
        glDeleteShader(renderer->flatColorShader.progId);
    }
    createShaderProgram(&renderer->flatColorShader, "shaders/flat_color.vs", "shaders/flat_color.fs");
    logOpenGLErrors();

    glBindAttribLocation(renderer->flatColorShader.progId, POS_ATTRIB_LOC, "position");
    glBindAttribLocation(renderer->flatColorShader.progId, NORM_ATTRIB_LOC, "normal");

    renderer->flatColorShader.projLoc = glGetUniformLocation(renderer->flatColorShader.progId, "projection");
    renderer->flatColorShader.viewLoc = glGetUniformLocation(renderer->flatColorShader.progId, "view");

    renderer->flatColorShader.posLoc = glGetUniformLocation(renderer->flatColorShader.progId, "entity_position");
    renderer->flatColorShader.sizeLoc = glGetUniformLocation(renderer->flatColorShader.progId, "entity_size");
    renderer->flatColorShader.rotLoc = glGetUniformLocation(renderer->flatColorShader.progId, "entity_rotation");
    renderer->flatColorShader.diffuseLoc = glGetUniformLocation(renderer->flatColorShader.progId, "entity_color");

    // 3d color flat shader
    if (renderer->groundShader.progId != 0) {
        glDeleteShader(renderer->groundShader.progId);
    }
    createShaderProgram(&renderer->groundShader, "shaders/ground.vs", "shaders/ground.fs");
    logOpenGLErrors();

    glBindAttribLocation(renderer->groundShader.progId, POS_ATTRIB_LOC, "position");
    glBindAttribLocation(renderer->groundShader.progId, NORM_ATTRIB_LOC, "normal");

    renderer->groundShader.projLoc = glGetUniformLocation(renderer->flatColorShader.progId, "projection");
    renderer->groundShader.viewLoc = glGetUniformLocation(renderer->flatColorShader.progId, "view");

    renderer->groundShader.posLoc = glGetUniformLocation(renderer->flatColorShader.progId, "entity_position");
    renderer->groundShader.sizeLoc = glGetUniformLocation(renderer->flatColorShader.progId, "entity_size");
    renderer->groundShader.rotLoc = glGetUniformLocation(renderer->flatColorShader.progId, "entity_rotation");
    renderer->groundShader.diffuseLoc = glGetUniformLocation(renderer->flatColorShader.progId, "entity_color");
}

void pushMeshPiece(Renderer* renderer, Shader* shader,
    GLuint vao, uint32 iCount,
    const Mat3& rot, const Vec3& size, const Vec3& pos, const Vec3& color)
{
    ASSERT(renderer->pieceCount < MAX_RENDER_PICES);

    RenderPiece* newPiece = &renderer->renderQueue[renderer->pieceCount++];

    newPiece->type = TRIMESH_COLOR;
    newPiece->shader = shader;
    newPiece->vao = vao;
    newPiece->iCount = iCount;
    newPiece->rotation = rot;
    newPiece->size = size;
    newPiece->position = pos;
    newPiece->color = color;
}

void pushMeshPieceWireframe(Renderer* renderer, Shader* shader,
    GLuint vao, uint32 iCount,
    const Mat3& rot, const Vec3& size, const Vec3& pos, const Vec3& color)
{
    ASSERT(renderer->pieceCount < MAX_RENDER_PICES);

    RenderPiece* newPiece = &renderer->renderQueue[renderer->pieceCount++];

    newPiece->type = TRIMESH_WIREFRAME;
    newPiece->shader = shader;
    newPiece->vao = vao;
    newPiece->iCount = iCount;
    newPiece->rotation = rot;
    newPiece->size = size;
    newPiece->position = pos;
    newPiece->color = color;
}

void pushMeshPieceTextured(Renderer* renderer, Shader* shader,
    GLuint vao, uint32 iCount, GLuint texId,
    const Mat3& rot, const Vec3& size, const Vec3& pos)
{
    ASSERT(renderer->pieceCount < MAX_RENDER_PICES);

    RenderPiece* newPiece = &renderer->renderQueue[renderer->pieceCount++];

    newPiece->type = TRIMESH_TEXTURE;
    newPiece->shader = shader;
    newPiece->vao = vao;
    newPiece->iCount = iCount;
    newPiece->texId = texId;
    newPiece->rotation = rot;
    newPiece->size = size;
    newPiece->position = pos;
}

void pushArrayPiece(Renderer* renderer, Shader* shader,
    GLuint vao, uint32 count, RenderPieceType type,
    const Mat3& rot, const Vec3& size, const Vec3& pos, const Vec3& color)
{
    ASSERT(renderer->pieceCount < MAX_RENDER_PICES);

    RenderPiece* newPiece = &renderer->renderQueue[renderer->pieceCount++];

    newPiece->type = type;
    newPiece->shader = shader;
    newPiece->vao = vao;
    newPiece->iCount = count;
    newPiece->rotation = rot;
    newPiece->size = size;
    newPiece->position = pos;
    newPiece->color = color;
}

void pushIndexedArrayPiece(Renderer* renderer, Shader* shader,
    GLuint vao, uint32 count, RenderPieceType type,
    const Mat3& rot, const Vec3& size, const Vec3& pos, const Vec3& color)
{
    ASSERT(renderer->pieceCount < MAX_RENDER_PICES);

    RenderPiece* newPiece = &renderer->renderQueue[renderer->pieceCount++];

    newPiece->type = type;
    newPiece->shader = shader;
    newPiece->vao = vao;
    newPiece->iCount = count;
    newPiece->rotation = rot;
    newPiece->size = size;
    newPiece->position = pos;
    newPiece->color = color;
}

void renderAll(Renderer* renderer, const Mat4& projection, const Mat4& view)
{

    uint32 count = renderer->pieceCount;
    RenderPiece* renderPieces = renderer->renderQueue;

    for (uint32 i = 0; i < count; i++) {
        RenderPiece* piece = renderPieces + i;
        Shader* shader = piece->shader;

        if (piece->type == TRIMESH_WIREFRAME) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        glUseProgram(shader->progId);
        glUniformMatrix4fv(shader->viewLoc, 1, true, view.data);
        glUniformMatrix4fv(shader->projLoc, 1, true, &projection.data[0]);

        glUniformMatrix3fv(shader->rotLoc, 1, true, piece->rotation.data);
        glUniform3f(shader->sizeLoc, piece->size.x, piece->size.y, piece->size.z);
        glUniform3f(shader->posLoc, piece->position.x, piece->position.y, piece->position.z);

        if (piece->type == TRIMESH_TEXTURE) {
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(shader->diffTexLoc, 0);
            glBindTexture(GL_TEXTURE_2D, piece->texId);
        }
        else {
            glUniform3f(shader->diffuseLoc, piece->color.x, piece->color.y, piece->color.z);
        }

        glBindVertexArray(piece->vao);
        if (piece->type == ARRAY_POINTS) {
            glDrawArrays(GL_POINTS, 0, piece->iCount);
        }
        else if (piece->type == ARRAY_LINE_LOOP) {
            glDrawArrays(GL_LINE_LOOP, 0, piece->iCount);
        }
        else if (piece->type == ARRAY_LINE_STRIP) {
            glDrawArrays(GL_LINE_STRIP, 0, piece->iCount);
        }
        else if (piece->type == INDEXED_ARRAY_LINE_LOOP) {
            glDrawElements(GL_LINE_LOOP, piece->iCount, GL_UNSIGNED_INT, 0);
        }
        else if (piece->type == INDEXED_ARRAY_LINES) {
            glDrawElements(GL_LINES, piece->iCount, GL_UNSIGNED_INT, 0);
        }
        else {
            glDrawElements(GL_TRIANGLES, piece->iCount, GL_UNSIGNED_INT, 0);
        }
    }

    renderer->pieceCount = 0;
}
