#include "Renderer.h"

void intializeRendererRessources(Renderer* renderer)
{
    reloadShaders(renderer);
    logOpenGLErrors();
    
    // 2 triangles -> 6 vertices + 6 tcs
    //uint32 rectIndices[12] = {0,1,2,0,2,3};
    renderer->rectVertices[0] = Vec2(-.5f,-.5f);
    renderer->rectVertices[1] = Vec2(-.5f,.5f);
    renderer->rectVertices[2] = Vec2(.5f,.5f);
    renderer->rectVertices[3] = Vec2(-.5f,-.5f);
    renderer->rectVertices[4] = Vec2(.5f,.5f);
    renderer->rectVertices[5] = Vec2(.5f,-.5f);
    
    renderer->rectVertices[6] = Vec2(.0f,0.f);
    renderer->rectVertices[7] = Vec2(0.f,1.f);
    renderer->rectVertices[8] = Vec2(1.f,1.f);
    renderer->rectVertices[9] = Vec2(0.f,0.f);
    renderer->rectVertices[10] = Vec2(1.f,1.f);
    renderer->rectVertices[11] = Vec2(1.f,0.f);
    
    renderer->spriteMesh.positions = renderer->rectVertices;
    //spriteMesh.indices = rectIndices;
    create2DVertexBuffer(&renderer->spriteMesh);
    logOpenGLErrors();
}

void reloadShaders(Renderer* renderer)
{
    // Sprite Shader
    glDeleteShader(renderer->spriteShader.progId);
    createShaderProgram(&renderer->spriteShader,"shaders/sprite.vs","shaders/sprite.fs");
    logOpenGLErrors();
    
    glBindAttribLocation(renderer->spriteShader.progId, 0, "pos2D");
    
    renderer->spriteShader.projLoc = glGetUniformLocation(renderer->spriteShader.progId, "projection");
    renderer->spriteShader.viewLoc = glGetUniformLocation(renderer->spriteShader.progId, "view");

    renderer->spriteShader.posLoc = glGetUniformLocation(renderer->spriteShader.progId, "entity_position");
    renderer->spriteShader.sizeLoc = glGetUniformLocation(renderer->spriteShader.progId, "entity_size");
    renderer->spriteShader.rotLoc = glGetUniformLocation(renderer->spriteShader.progId, "entity_rotation");

    renderer->spriteShader.texLoc = glGetUniformLocation(renderer->spriteShader.progId, "sprite_texture");
    
    // 3d textured shader
    glDeleteShader(renderer->flatTextureShader.progId);
    createShaderProgram(&renderer->flatTextureShader,"shaders/flat_textured.vs","shaders/flat_textured.fs");
    logOpenGLErrors();
    
    glBindAttribLocation(renderer->flatTextureShader.progId, 0, "pos");
    
    renderer->flatTextureShader.projLoc = glGetUniformLocation(renderer->flatTextureShader.progId, "projection");
    renderer->flatTextureShader.viewLoc = glGetUniformLocation(renderer->flatTextureShader.progId, "view");
    
    renderer->flatTextureShader.posLoc = glGetUniformLocation(renderer->flatTextureShader.progId, "entity_position");
    renderer->flatTextureShader.sizeLoc = glGetUniformLocation(renderer->flatTextureShader.progId, "entity_size");
    renderer->flatTextureShader.rotLoc = glGetUniformLocation(renderer->flatTextureShader.progId, "entity_rotation");
    
    renderer->flatTextureShader.texLoc = glGetUniformLocation(renderer->flatTextureShader.progId, "sprite_texture");
    
    // 3d flat colord shader
    glDeleteShader(renderer->vertexDiffuseShader.progId);
    createShaderProgram(&renderer->vertexDiffuseShader,"shaders/vertex_diffuse.vs","shaders/vertex_diffuse.fs");
    logOpenGLErrors();
    
    glBindAttribLocation(renderer->vertexDiffuseShader.progId, 0, "position");
    glBindAttribLocation(renderer->vertexDiffuseShader.progId, 1, "normal");
    
    renderer->vertexDiffuseShader.projLoc = glGetUniformLocation(renderer->vertexDiffuseShader.progId, "projection");
    renderer->vertexDiffuseShader.viewLoc = glGetUniformLocation(renderer->vertexDiffuseShader.progId, "view");
    
    renderer->vertexDiffuseShader.posLoc = glGetUniformLocation(renderer->vertexDiffuseShader.progId, "entity_position");
    renderer->vertexDiffuseShader.sizeLoc = glGetUniformLocation(renderer->vertexDiffuseShader.progId, "entity_size");
    renderer->vertexDiffuseShader.rotLoc = glGetUniformLocation(renderer->vertexDiffuseShader.progId, "entity_rotation");
    
    // 3d Textured shader
    glDeleteShader(renderer->textureDiffuseShader.progId);
    createShaderProgram(&renderer->textureDiffuseShader,"shaders/texture_diffuse.vs","shaders/texture_diffuse.fs");
    logOpenGLErrors();
    
    renderer->textureDiffuseShader.projLoc = glGetUniformLocation(renderer->textureDiffuseShader.progId, "projection");
    renderer->textureDiffuseShader.viewLoc = glGetUniformLocation(renderer->textureDiffuseShader.progId, "view");
    
    renderer->textureDiffuseShader.posLoc = glGetUniformLocation(renderer->textureDiffuseShader.progId, "entity_position");
    renderer->textureDiffuseShader.sizeLoc = glGetUniformLocation(renderer->textureDiffuseShader.progId, "entity_size");
    renderer->textureDiffuseShader.rotLoc = glGetUniformLocation(renderer->textureDiffuseShader.progId, "entity_rotation");
    
    renderer->textureDiffuseShader.diffuseLoc = glGetUniformLocation(renderer->textureDiffuseShader.progId, "diffuse");
    

}

