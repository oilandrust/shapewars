#ifndef OPENGLCLIENTH
#define OPENGLCLIENTH

#include "GetTheFlag.h"
#include "Vec2.h"

#include <GL/glew.h>
#include <OpenGL/OpenGL.h>

struct Shader {
    GLuint progId;
    
    // attributes
    GLint vertexAttrLoc;
    
    // uniforms
    GLint projLoc;
    GLint posLoc;
    GLint sizeLoc;
};

struct Mesh {
    Vec2* positions;
    uint32* indices;
    GLuint vboId;
    GLuint iboId;
};

bool createShaderProgram(Shader* shader, const char* vsShaderFilename, const char* fsShaderFilename);

void createVertexAndIndexBuffer(Mesh *mesh);

inline void ortho(real32* mat, real32 left, real32 right, real32 bottom, real32 top, real32 near, real32 far)
{
    real32 rli = 1.f/(right-left);
    real32 tbi = 1.f/(top - bottom);
    real32 fni = 1.f/(far - near);
    // diag
    mat[0] = 2.f*rli;
    mat[5] = 2.f*tbi;
    mat[10] = -2.f*fni;
    mat[15] = 1;
    
    // trans
    mat[3] = -rli*(right+left);
    mat[7] = -tbi*(top+bottom);
    mat[11] = -fni*(far+near);
}

static void printShaderLog(GLuint shader);
static void printProgramLog(GLuint program);


#endif
