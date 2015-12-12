#ifndef RENDERERH
#define RENDERERH

#include "GetTheFlag.h"
#include "Vec2.h"
#include "OpenGLClient.h"

struct Renderer {
    Shader flatDiffShader;
    Shader texDiffShader;
};

void intializeRendererRessources(Renderer* renderer);

void rendererBeginFrame(Renderer* renderer);

void reloadShaders(Renderer* renderer);


#endif