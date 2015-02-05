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

#define logOpenGLErrors() _logOpenGLErrors(__FILE__,__LINE__)

struct Shader {
    GLuint progId;
    
    // attributes
    GLint vertexAttrLoc;
    
    // uniforms
    GLint projLoc;
    GLint viewLoc;
    GLint posLoc;
    GLint sizeLoc;
    GLint texLoc;
    GLint diffuseLoc;
    GLint rotLoc;
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
    SDL_Surface* surface;
};

void _logOpenGLErrors(const char *file, int32 line);

void loadTexture(Texture* tex, const char* filename);

bool createTexture(Texture* texture);

bool createShaderProgram(Shader* shader, const char* vsShaderFilename, const char* fsShaderFilename);

bool create2DVertexBuffer(Mesh2 *mesh);

GLuint create3DVertexArray(Mesh3D *mesh);

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

inline void perspective(real32* mat, real32 fovy, real32 aspect, real32 near, real32 far)
{
    real32 f = 1.f/tanf(.5f*fovy*PI/180.f);
    real32 iNearFar = .1f/(near-far);
    mat[0] = f/aspect;
    mat[5] = f;
    mat[10] = iNearFar * (far+near);
    mat[11] = 2.f * far * near * iNearFar;
    mat[14] = -1.f;
}

void printShaderLog(GLuint shader);
void printProgramLog(GLuint program);


#endif
