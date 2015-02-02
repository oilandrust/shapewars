#ifndef RENDERERH
#define RENDERERH

#include "GetTheFlag.h"
#include "Vec2.h"
#include "OpenGLClient.h"

struct Renderer {
    Vec2 rectVertices[12];
    Mesh2 spriteMesh;
    
    Shader spriteShader;
    Shader flatTextureShader;
    Shader vertexDiffuseShader;
    Shader textureDiffuseShader;
};

void intializeRendererRessources(Renderer* renderer);

void reloadShaders(Renderer* renderer);


#endif