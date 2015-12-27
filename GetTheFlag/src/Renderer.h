#ifndef RENDERERH
#define RENDERERH

#include "GetTheFlag.h"
#include "OpenGLClient.h"
#include "Vec2.h"

struct Renderer {
    Shader flatDiffShader;
    Shader texDiffShader;
    Shader flatColorShader;
};

void intializeRendererRessources(Renderer* renderer);

void rendererBeginFrame(Renderer* renderer);

void reloadShaders(Renderer* renderer);

#endif