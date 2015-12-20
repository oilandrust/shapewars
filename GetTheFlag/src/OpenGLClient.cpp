#include "OpenGLClient.h"

#include <cstdio>
#include <stb_image.h>

bool createTexture(Texture* texture, GLint format)
{
    glGenTextures(1, &texture->texId);
    glBindTexture(GL_TEXTURE_2D, texture->texId);

    logOpenGLErrors();

    glTexImage2D(GL_TEXTURE_2D,
        0,
        format,
        texture->width, texture->height, 0,
        format,
        GL_UNSIGNED_BYTE,
        texture->data);

    logOpenGLErrors();

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    logOpenGLErrors();
    return true;
}


void updateTextureData(Texture* texture, GLint format, void* data)
{
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 format,
                 texture->width, texture->height, 0,
                 format,
                 GL_UNSIGNED_BYTE,
                 data);
}

void loadTexture(Texture* tex, const char* filename)
{
    int32 w, h, componentPerPixels;
    uint8* data = stbi_load(filename, &w, &h, &componentPerPixels, 0);
    ASSERT(data);
    ASSERT(componentPerPixels == 4);

    tex->data = data;
    tex->width = w;
    tex->height = h;

    createTexture(tex, GL_RGBA);
}

GLuint create3DIndexedVertexArray(Mesh3D* mesh)
{
    //Create VBO
    GLuint vbo, nbo, cbo, tbo;
    {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, mesh->vCount * sizeof(Vec3), mesh->positions, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        if (mesh->normals) {
            glGenBuffers(1, &nbo);
            glBindBuffer(GL_ARRAY_BUFFER, nbo);
            glBufferData(GL_ARRAY_BUFFER, mesh->vCount * sizeof(Vec3), mesh->normals, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        if (mesh->colors) {
            glGenBuffers(1, &cbo);
            glBindBuffer(GL_ARRAY_BUFFER, cbo);
            glBufferData(GL_ARRAY_BUFFER, mesh->vCount * sizeof(Vec3), mesh->colors, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        if (mesh->uvs) {
            glGenBuffers(1, &tbo);
            glBindBuffer(GL_ARRAY_BUFFER, tbo);
            glBufferData(GL_ARRAY_BUFFER, mesh->vCount * sizeof(Vec2), mesh->uvs, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }

    // Element Buffer
    GLuint ibo;
    {
        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * mesh->fCount * sizeof(uint32), mesh->indices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    // Create the vao
    GLuint vao;
    {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        if (mesh->normals) {
            glBindBuffer(GL_ARRAY_BUFFER, nbo);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        }

        if (mesh->colors) {
            glBindBuffer(GL_ARRAY_BUFFER, cbo);
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        }

        if (mesh->uvs) {
            glBindBuffer(GL_ARRAY_BUFFER, tbo);
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    return vao;
}

GLuint create3DVertexArray(Mesh3D* mesh)
{
    //Create VBO
    GLuint vbo, nbo, cbo, tbo;
    {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, mesh->vCount * sizeof(Vec3), mesh->positions, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        if (mesh->normals) {
            glGenBuffers(1, &nbo);
            glBindBuffer(GL_ARRAY_BUFFER, nbo);
            glBufferData(GL_ARRAY_BUFFER, mesh->vCount * sizeof(Vec3), mesh->normals, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        
        if (mesh->colors) {
            glGenBuffers(1, &cbo);
            glBindBuffer(GL_ARRAY_BUFFER, cbo);
            glBufferData(GL_ARRAY_BUFFER, mesh->vCount * sizeof(Vec3), mesh->colors, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        
        if (mesh->uvs) {
            glGenBuffers(1, &tbo);
            glBindBuffer(GL_ARRAY_BUFFER, tbo);
            glBufferData(GL_ARRAY_BUFFER, mesh->vCount * sizeof(Vec2), mesh->uvs, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }
    logOpenGLErrors();
    
    // Create the vao
    GLuint vao;
    {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        
        if (mesh->normals) {
            glBindBuffer(GL_ARRAY_BUFFER, nbo);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        }
        
        if (mesh->colors) {
            glBindBuffer(GL_ARRAY_BUFFER, cbo);
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        }
        
        if (mesh->uvs) {
            glBindBuffer(GL_ARRAY_BUFFER, tbo);
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    logOpenGLErrors();
    return vao;
}

GLuint create3DVertexArray(Vec3* data, uint32 count, uint32* indices, uint32 iCount)
{
    //Create VBO
    GLuint vbo;
    {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vec3), data, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    logOpenGLErrors();
    
    
    // Element Buffer
    GLuint ibo;
    {
        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, iCount * sizeof(uint32), indices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    
    // Create the vao
    GLuint vao;
    {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    logOpenGLErrors();
    return vao;
}

GLuint createShader(GLenum shaderType, const char* filename)
{
    char shaderSource[4096];
    char inChar;
    FILE* shaderFile;
    int i = 0;

    GLuint shader = 0;
    shaderFile = fopen(filename, "r");
    if (shaderFile) {
        while (fscanf(shaderFile, "%c", &inChar) > 0) {
            shaderSource[i++] = inChar;
        }
        shaderSource[i - 1] = '\0';
        fclose(shaderFile);

        shader = glCreateShader(shaderType);
        const char* source = shaderSource;
        glShaderSource(shader, 1, &source, 0);
        glCompileShader(shader);

        GLint success = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (success != GL_TRUE) {
            printf("Unable to compile vertex shader %d!\n", shader);
            puts(shaderSource);
            printShaderLog(shader);
            success = false;
        }
    }
    else {
        printf("Unable to open shader %s\n", filename);
    }
    return shader;
}

bool createShaderProgram(Shader* shader, const char* vsShaderFilename, const char* fsShaderFilename)
{
    shader->progId = glCreateProgram();

    GLuint vs = createShader(GL_VERTEX_SHADER, vsShaderFilename);
    glAttachShader(shader->progId, vs);

    GLuint fs = createShader(GL_FRAGMENT_SHADER, fsShaderFilename);
    glAttachShader(shader->progId, fs);

    glLinkProgram(shader->progId);

    GLint success = GL_TRUE;
    glGetProgramiv(shader->progId, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        printf("Error linking program %d!\n", shader->progId);
        printProgramLog(shader->progId);
    }
    logOpenGLErrors();

    return success;
}

void printShaderLog(GLuint shader)
{
    if (glIsShader(shader)) {
        //Shader log length
        int infoLogLength = 0;
        int maxLength = infoLogLength;

        //Get info string length
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        char* infoLog = new char[maxLength];
        glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
        if (infoLogLength > 0) {
            printf("%s\n", infoLog);
        }
        delete[] infoLog;
    }
    else {
        printf("Name %d is not a shader\n", shader);
    }
}

void printProgramLog(GLuint program)
{
    if (glIsProgram(program)) {
        int infoLogLength = 0;
        int maxLength = infoLogLength;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        char* infoLog = new char[maxLength];

        glGetProgramInfoLog(program, maxLength, &infoLogLength, infoLog);
        if (infoLogLength > 0) {
            printf("%s\n", infoLog);
        }
        delete[] infoLog;
    }
    else {
        printf("Name %d is not a program\n", program);
    }
}

void _logOpenGLErrors(const char* file, int32 line)
{
    GLenum err(glGetError());

    while (err != GL_NO_ERROR) {
        const char* error;
        switch (err) {
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }

        printf("GL_%s at line %d of %s\n", error, line, file);
        err = glGetError();
    }
}
