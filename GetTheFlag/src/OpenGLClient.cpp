#include "OpenGLClient.h"
#include "cstdio"


bool createTexture(Texture* texture)
{
    glGenTextures(1, &texture->texId);
    glBindTexture(GL_TEXTURE_2D, texture->texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 texture->width, texture->height, 0,
                 GL_RGBA,
                 GL_UNSIGNED_INT_8_8_8_8,
                 texture->data);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    logOpenGLErrors();
    return true;
}

bool createVertexAndIndexBuffer(Mesh *mesh)
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    //Create VBO
    glGenBuffers(1, &mesh->vboId);
    glBindVertexArray(mesh->vboId);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vboId);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(Vec2), mesh->positions, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    //Create IBO
    glGenBuffers(1, &mesh->iboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->iboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(uint32), mesh->indices, GL_STATIC_DRAW);
    
    ASSERT(mesh->vboId != GL_INVALID_VALUE, "Generatin VBO failed");
    ASSERT(mesh->iboId != GL_INVALID_VALUE, "Generatin IBO failed");
    
    return !glGetError();
}

GLuint createShader(GLenum shaderType, const char* filename)
{
    char shaderSource[4096];
    char inChar;
    FILE *shaderFile;
    int i = 0;
    
    shaderFile = fopen(filename, "r");
    while(fscanf(shaderFile,"%c",&inChar) > 0)
    {
        shaderSource[i++] = inChar;
    }
    shaderSource[i - 1] = '\0';
    fclose(shaderFile);
    
    GLuint shader = glCreateShader(shaderType);
    const char* source = shaderSource;
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    
    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(success != GL_TRUE)
    {
        printf("Unable to compile vertex shader %d!\n", shader);
        puts(shaderSource);
        printShaderLog(shader);
        success = false;
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
    if(success != GL_TRUE)
    {
        printf("Error linking program %d!\n", shader->progId);
        printProgramLog(shader->progId);
    }
    
    // Get Vertex Location
    shader->vertexAttrLoc = glGetAttribLocation(shader->progId, "pos2D");
    if(shader->vertexAttrLoc == -1)
    {
        printf( "pos2D is not a valid glsl program variable!\n" );
        success = false;
    }

    return success;
}

void printShaderLog(GLuint shader)
{
    if(glIsShader(shader))
    {
        //Shader log length
        int infoLogLength = 0;
        int maxLength = infoLogLength;
        
        //Get info string length
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
        
        char* infoLog = new char[ maxLength ];
        glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
        if(infoLogLength > 0)
        {
            printf( "%s\n", infoLog );
        }
        delete[] infoLog;
    }
    else
    {
        printf( "Name %d is not a shader\n", shader );
    }
}

void printProgramLog(GLuint program)
{
    if(glIsProgram(program))
    {
        int infoLogLength = 0;
        int maxLength = infoLogLength;
        
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        
        char* infoLog = new char[maxLength];
        
        glGetProgramInfoLog(program, maxLength, &infoLogLength, infoLog);
        if(infoLogLength > 0)
        {
            printf("%s\n", infoLog);
        }
        delete[] infoLog;
    }
    else
    {
        printf( "Name %d is not a program\n", program );
    }
}

void _logOpenGLErrors(const char *file, int32 line)
{
    GLenum err (glGetError());
    
    while(err!=GL_NO_ERROR)
    {
        const char* error;
        switch(err) {
            case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
            case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
            case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
            case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
        }
        
        printf("GL_%s at line %d of %s\n",error,line,file);
        err=glGetError();
    }
}

