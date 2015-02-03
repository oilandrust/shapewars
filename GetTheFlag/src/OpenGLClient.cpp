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

SDL_Surface* loadBitmap(const char* filename)
{
    SDL_RWops *rwop;
    rwop = SDL_RWFromFile(filename, "rb");
    if(!rwop)
    {
        printf("SDL_RWFromFile: %s\n", IMG_GetError());
        ASSERT(false, IMG_GetError());
    }
    SDL_Surface* bitmap = IMG_LoadPNG_RW(rwop);
    if(!bitmap)
    {
        printf("IMG_LoadPNG_RW: %s\n", IMG_GetError());
        printf("Error loading: %s\n", filename);
        ASSERT(false, IMG_GetError());
    }
    return bitmap;
}

void loadTexture(Texture* tex, const char* filename)
{
    SDL_Surface* surface = loadBitmap(filename);
    
    tex->data = surface->pixels;
    tex->width = surface->w;
    tex->height = surface->h;
    tex->surface = surface;
    createTexture(tex);
}

bool create2DVertexBuffer(Mesh2 *mesh)
{
    
    //Create VBO
    {
        glGenBuffers(1, &mesh->vboId);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vboId);
        glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(Vec2), mesh->positions, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    // Create the vao
    {
        glGenVertexArrays(1, &mesh->vaoId);
        glBindVertexArray(mesh->vaoId);
        
            // Bind it to the vbo
            glBindBuffer(GL_ARRAY_BUFFER, mesh->vboId);
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(6 * sizeof(Vec2)));
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        glBindVertexArray(0);
    }
    return !glGetError();
}

GLuint create3DVertexArray(Mesh3D *mesh)
{
    //Create VBO
    GLuint vbo, nbo, cbo, tbo;
    {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, mesh->vCount * sizeof(Vec3), mesh->positions, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        if(mesh->normals)
        {
            glGenBuffers(1, &nbo);
            glBindBuffer(GL_ARRAY_BUFFER, nbo);
            glBufferData(GL_ARRAY_BUFFER, mesh->vCount * sizeof(Vec3), mesh->normals, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        
        if(mesh->colors)
        {
            glGenBuffers(1, &cbo);
            glBindBuffer(GL_ARRAY_BUFFER, cbo);
            glBufferData(GL_ARRAY_BUFFER, mesh->vCount * sizeof(Vec3), mesh->colors, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        
        if(mesh->uvs)
        {
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
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*mesh->fCount * sizeof(uint32), mesh->indices, GL_STATIC_DRAW);
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
        
        if(mesh->normals)
        {
            glBindBuffer(GL_ARRAY_BUFFER, nbo);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        }
        
        if(mesh->colors)
        {
            glBindBuffer(GL_ARRAY_BUFFER, cbo);
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        }
        
        if(mesh->uvs)
        {
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
//    shader->vertexAttrLoc = glGetAttribLocation(shader->progId, "pos");
//    if(shader->vertexAttrLoc == -1)
//    {
//        printf( "pos is not a valid glsl program variable!\n" );
//        success = false;
//    }

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



