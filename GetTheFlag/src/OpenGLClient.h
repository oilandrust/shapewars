#ifndef OPENGLCLIENTH
#define OPENGLCLIENTH

#include "GetTheFlag.h"
#include "Vec2.h"
#include "Vec3.h"
#include "Mesh.h"

#include <SDL.h>
#include <cmath>

#include <GL/glew.h>
#include <OpenGL/OpenGL.h>

#define logOpenGLErrors() _logOpenGLErrors(__FILE__, __LINE__)

struct Shader {
    GLuint progId;

    // attributes
    GLint vertexAttrLoc;

    // uniforms
    GLint projLoc;
    GLint viewLoc;
    GLint posLoc;
    GLint sizeLoc;
    GLint diffuseLoc;
    GLint rotLoc;
    GLint diffTexLoc;
};

struct Mesh2 {
    GLuint vaoId;
    GLuint vboId;

    Vec2* positions;
};

struct Texture {
    GLuint texId;
    uint32 width;
    uint32 height;

    void* data;
};

void _logOpenGLErrors(const char* file, int32 line);

void loadTexture(Texture* tex, const char* filename);

bool createTexture(Texture* texture, GLint format);

void updateTextureData(Texture* texture, GLint format, void* data);

bool createShaderProgram(Shader* shader, const char* vsShaderFilename, const char* fsShaderFilename);

bool create2DVertexBuffer(Mesh2* mesh);

GLuint create3DIndexedVertexArray(Mesh3D* mesh);

GLuint create3DVertexArray(Mesh3D* mesh);

void printShaderLog(GLuint shader);
void printProgramLog(GLuint program);

#endif
