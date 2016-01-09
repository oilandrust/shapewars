#ifndef OPENGLCLIENTH
#define OPENGLCLIENTH

#include "Mesh.h"
#include "ShapeWars.h"
#include "Vec2.h"
#include "Vec3.h"

#include <SDL.h>
#include <cmath>

#include <GL/glew.h>
#include <OpenGL/OpenGL.h>

struct Shader {
    GLuint progId = 0;

    // uniforms
    GLint projLoc;
    GLint viewLoc;
    GLint posLoc;
    GLint sizeLoc;
    GLint diffuseLoc;
    GLint rotLoc;
    GLint diffTexLoc;
};

#define POS_ATTRIB_LOC 0
#define NORM_ATTRIB_LOC 1
#define COL_ATTRIB_LOC 2
#define UV_ATTRIB_LOC 3

struct Texture {
    GLuint texId;
    uint32 width;
    uint32 height;

    void* data;
};

/**
 * Textue
 */
void loadTexture(Texture* tex, const char* filename);

bool createTexture(Texture* texture, GLint format);

void updateTextureData(Texture* texture, GLint format, void* data);

/**
 * Shader
 */
bool createShaderProgram(Shader* shader, const char* vsShaderFilename, const char* fsShaderFilename);

void printShaderLog(GLuint shader);

void printProgramLog(GLuint program);

/**
 * Mesh
 */

// VBO
template <class Vec>
GLuint createBufferObject(Vec* data, uint32 count, GLenum mode = GL_STATIC_DRAW)
{
    GLuint boId;
    glGenBuffers(1, &boId);
    glBindBuffer(GL_ARRAY_BUFFER, boId);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vec), data, mode);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return boId;
}

template <class Vec>
void updateBufferObject(GLuint bufferID, Vec* data, uint32 count)
{
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(Vec), data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void bindAttribBuffer(GLuint buffer, GLuint loc, uint32 size);

// VAO
GLuint createVertexArray(Vec3* data, uint32 count);

GLuint createVertexArray(GLuint vbo);

GLuint createIndexedVertexArray(Mesh3D* mesh);

GLuint createIndexedVertexArray(Vec3* data, uint32 count, uint32* indices, uint32 iCount);

/**
 * Log
 */
#define logOpenGLErrors() _logOpenGLErrors(__FILE__, __LINE__)

void _logOpenGLErrors(const char* file, int32 line);

#endif
