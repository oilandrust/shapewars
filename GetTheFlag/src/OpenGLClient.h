#ifndef OPENGLCLIENTH
#define OPENGLCLIENTH

#include "GetTheFlag.h"
#include "Mesh.h"
#include "Vec2.h"
#include "Vec3.h"

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

#define POS_ATTRIB_LOC 0
#define NORM_ATTRIB_LOC 1
#define COL_ATTRIB_LOC 2
#define UV_ATTRIB_LOC 3

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

GLuint create3DIndexedVertexArray(Mesh3D* mesh);

GLuint create3DVertexArray(Vec3* data, uint32 count, uint32* indices, uint32 iCount);

void printShaderLog(GLuint shader);
void printProgramLog(GLuint program);

#endif
