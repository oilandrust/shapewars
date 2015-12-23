#include "Renderer.h"

void intializeRendererRessources(Renderer* renderer)
{
    reloadShaders(renderer);
    logOpenGLErrors();
}


void rendererBeginFrame(Renderer* renderer)
{
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);
    
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
    glDeleteShader(renderer->texDiffShader.progId);
    createShaderProgram(&renderer->texDiffShader,"shaders/texture_diffuse.vs","shaders/texture_diffuse.fs");
    logOpenGLErrors();
    
    glBindAttribLocation(renderer->texDiffShader.progId, 0, "position");
    glBindAttribLocation(renderer->texDiffShader.progId, 1, "normal");
    glBindAttribLocation(renderer->texDiffShader.progId, 2, "uv");
    
    renderer->texDiffShader.projLoc = glGetUniformLocation(renderer->texDiffShader.progId, "projection");
    renderer->texDiffShader.viewLoc = glGetUniformLocation(renderer->texDiffShader.progId, "view");
    renderer->texDiffShader.diffTexLoc = glGetUniformLocation(renderer->texDiffShader.progId, "diffuse");
    
    renderer->texDiffShader.posLoc = glGetUniformLocation(renderer->texDiffShader.progId, "entity_position");
    renderer->texDiffShader.sizeLoc = glGetUniformLocation(renderer->texDiffShader.progId, "entity_size");
    renderer->texDiffShader.rotLoc = glGetUniformLocation(renderer->texDiffShader.progId, "entity_rotation");
    
    // 3d color diffuse shader
    glDeleteShader(renderer->flatDiffShader.progId);
    createShaderProgram(&renderer->flatDiffShader,"shaders/flat_diffuse.vs","shaders/flat_diffuse.fs");
    logOpenGLErrors();
    
    glBindAttribLocation(renderer->flatDiffShader.progId, 0, "position");
    glBindAttribLocation(renderer->flatDiffShader.progId, 1, "normal");
    
    renderer->flatDiffShader.projLoc = glGetUniformLocation(renderer->flatDiffShader.progId, "projection");
    renderer->flatDiffShader.viewLoc = glGetUniformLocation(renderer->flatDiffShader.progId, "view");
    
    renderer->flatDiffShader.posLoc = glGetUniformLocation(renderer->flatDiffShader.progId, "entity_position");
    renderer->flatDiffShader.sizeLoc = glGetUniformLocation(renderer->flatDiffShader.progId, "entity_size");
    renderer->flatDiffShader.rotLoc = glGetUniformLocation(renderer->flatDiffShader.progId, "entity_rotation");
    renderer->flatDiffShader.diffuseLoc = glGetUniformLocation(renderer->flatDiffShader.progId, "entity_color");

    // 3d color flat shader
    glDeleteShader(renderer->flatColorShader.progId);
    createShaderProgram(&renderer->flatColorShader,"shaders/flat_color.vs","shaders/flat_color.fs");
    logOpenGLErrors();
    
    glBindAttribLocation(renderer->flatColorShader.progId, 0, "position");
    glBindAttribLocation(renderer->flatColorShader.progId, 1, "normal");
    
    renderer->flatColorShader.projLoc = glGetUniformLocation(renderer->flatColorShader.progId, "projection");
    renderer->flatColorShader.viewLoc = glGetUniformLocation(renderer->flatColorShader.progId, "view");
    
    renderer->flatColorShader.posLoc = glGetUniformLocation(renderer->flatColorShader.progId, "entity_position");
    renderer->flatColorShader.sizeLoc = glGetUniformLocation(renderer->flatColorShader.progId, "entity_size");
    renderer->flatColorShader.rotLoc = glGetUniformLocation(renderer->flatColorShader.progId, "entity_rotation");
    renderer->flatColorShader.diffuseLoc = glGetUniformLocation(renderer->flatColorShader.progId, "entity_color");

}

