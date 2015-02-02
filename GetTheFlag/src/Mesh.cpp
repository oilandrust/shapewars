#include "Mesh.h"
#include <cstdio>

void* createCube(Mesh3D* mesh, void* memoryPool)
{
    uint32 vCount = 24;
    uint32 fCount = 12;
    
    mesh->vCount = vCount;
    mesh->fCount = fCount;
  
    Vec3* p = (Vec3*)memoryPool;
    Vec3* n = p + vCount;
    Vec2* uv = (Vec2*)(n+ vCount);
    uint32* i = (uint32*)(uv + vCount);
    
    mesh->positions = p;
    mesh->normals = n;
    mesh->uvs = uv;
    mesh->indices = i;
    
    // TODO: Check the orientation of all faces with backface culling
    
    uint32 f = 0;
    // +Z
    *p++ = Vec3(-.5f,-.5f,.5f);  *n++ = Vec3(0,0,1); *uv++ = Vec2(0.f,0.f);
    *p++ = Vec3(-.5f, .5f,.5f);  *n++ = Vec3(0,0,1); *uv++ = Vec2(0.f,1.f);
    *p++ = Vec3( .5f, .5f,.5f);  *n++ = Vec3(0,0,1); *uv++ = Vec2(1.f,1.f);
    *p++ = Vec3( .5f,-.5f,.5f);  *n++ = Vec3(0,0,1); *uv++ = Vec2(1.f,0.f);
    *i++ = f; *i++ = f+1; *i++ = f+2;
    *i++ = f; *i++ = f+2; *i++ = f+3;
    f += 4;
    
    // -Z
    *p++ = Vec3(-.5f,-.5f,-.5f);  *n++ = Vec3(0,0,-1); *uv++ = Vec2(0.f,0.f);
    *p++ = Vec3( .5f,-.5f,-.5f);  *n++ = Vec3(0,0,-1); *uv++ = Vec2(0.f,1.f);
    *p++ = Vec3( .5f, .5f,-.5f);  *n++ = Vec3(0,0,-1); *uv++ = Vec2(1.f,1.f);
    *p++ = Vec3(-.5f, .5f,-.5f);  *n++ = Vec3(0,0,-1); *uv++ = Vec2(1.f,0.f);
    *i++ = f; *i++ = f+1; *i++ = f+2;
    *i++ = f; *i++ = f+2; *i++ = f+3;
    f += 4;
    
    // +X
    *p++ = Vec3(.5f,-.5f,-.5f);  *n++ = Vec3(1,0,0); *uv++ = Vec2(0.f,0.f);
    *p++ = Vec3(.5f, .5f,-.5f);  *n++ = Vec3(1,0,0); *uv++ = Vec2(0.f,1.f);
    *p++ = Vec3(.5f, .5f, .5f);  *n++ = Vec3(1,0,0); *uv++ = Vec2(1.f,1.f);
    *p++ = Vec3(.5f,-.5f, .5f);  *n++ = Vec3(1,0,0); *uv++ = Vec2(1.f,0.f);
    *i++ = f; *i++ = f+1; *i++ = f+2;
    *i++ = f; *i++ = f+2; *i++ = f+3;
    f += 4;
    
    // -X
    *p++ = Vec3(-.5f,-.5f,-.5f);  *n++ = Vec3(-1,0,0); *uv++ = Vec2(0.f,0.f);
    *p++ = Vec3(-.5f,-.5f, .5f);  *n++ = Vec3(-1,0,0); *uv++ = Vec2(0.f,1.f);
    *p++ = Vec3(-.5f, .5f, .5f);  *n++ = Vec3(-1,0,0); *uv++ = Vec2(1.f,1.f);
    *p++ = Vec3(-.5f, .5f,-.5f);  *n++ = Vec3(-1,0,0); *uv++ = Vec2(1.f,0.f);
    *i++ = f; *i++ = f+1; *i++ = f+2;
    *i++ = f; *i++ = f+2; *i++ = f+3;
    f += 4;
    
    // -Y
    *p++ = Vec3(-.5f,-.5f,-.5f);  *n++ = Vec3(0,-1,0); *uv++ = Vec2(0.f,0.f);
    *p++ = Vec3(-.5f,-.5f, .5f);  *n++ = Vec3(0,-1,0); *uv++ = Vec2(0.f,1.f);
    *p++ = Vec3( .5f,-.5f, .5f);  *n++ = Vec3(0,-1,0); *uv++ = Vec2(1.f,1.f);
    *p++ = Vec3( .5f,-.5f,-.5f);  *n++ = Vec3(0,-1,0); *uv++ = Vec2(1.f,0.f);
    *i++ = f; *i++ = f+1; *i++ = f+2;
    *i++ = f; *i++ = f+2; *i++ = f+3;
    f += 4;
    
    // +Y
    *p++ = Vec3(-.5f,.5f,-.5f);  *n++ = Vec3(0,1,0); *uv++ = Vec2(0.f,0.f);
    *p++ = Vec3(-.5f,.5f, .5f);  *n++ = Vec3(0,1,0); *uv++ = Vec2(0.f,1.f);
    *p++ = Vec3( .5f,.5f, .5f);  *n++ = Vec3(0,1,0); *uv++ = Vec2(1.f,1.f);
    *p++ = Vec3( .5f,.5f,-.5f);  *n++ = Vec3(0,1,0); *uv++ = Vec2(1.f,0.f);
    *i++ = f; *i++ = f+1; *i++ = f+2;
    *i++ = f; *i++ = f+2; *i++ = f+3;
    f += 4;
    
    return i+3*fCount;
}

void* loadObjMesh(Mesh3D* mesh, void* memoryPool, const char* filename, bool perVertexColors)
{
    FILE* file = fopen(filename, "r");
    if(file)
    {
        void* end = memoryPool;
        
        Vec3* p = 0;
        Vec3* n = 0;
        Vec3* c = 0;
        Vec2* uv = 0;
        uint32* i = 0;
        
        uint32 vCount = 0;
        uint32 fCount = 0;
        uint32 nCount = 0;
        uint32 uCount = 0;
    
        
        // In case positions and normals are interleaved, we need to know the vertex count
        // before loading the data so we can pack tightly:
        // ppppnnnnccccuuuuuiiiiiii
        char line[256];
        while (fgets(line, sizeof(line), file))
        {
            if(line[0] == 'v' && line[1] == ' ')
            {
                vCount++;
            }
            else if(line[0] == 'v' && line[1] == 'n')
            {
                nCount++;
            }
            else if(line[0] == 'v' && line[1] == 't')
            {
                uCount++;
            }
            else if(line[0] == 'f')
            {
                fCount++;
            }
        }
        ASSERT(vCount == nCount || nCount == 0, "Problem in file format");
        //ASSERT(vCount == uCount || uCount == 0, "Problem in file format");
        fseek(file,0,0);
        
        // Set all the pointers
        p = (Vec3*)end;
        end = (Vec3*)end + vCount;
        if(nCount > 0)
        {
            n = (Vec3*)end;
            end = (Vec3*)end + vCount;
        }
        if (perVertexColors)
        {
            c = (Vec3*)end;
            end = (Vec3*)end + vCount;
        }
        if(uCount > 0)
        {
            uv = (Vec2*)end;
            end = (Vec2*)end + vCount;
        }
        i = (uint32*)end;
        end = (uint32*)end + 3*fCount;
        
        mesh->vCount = vCount;
        mesh->fCount = fCount;
        
        mesh->positions = p;
        mesh->normals = n;
        mesh->colors = c;
        mesh->uvs = uv;
        mesh->indices = i;

        while (fgets(line, sizeof(line), file))
        {
            if(line[0] == 'v' && line[1] == ' ')
            {
                if(perVertexColors)
                {
                    real32 x,y,z,r,g,b;
                    sscanf(line, "v %f %f %f %f %f %f\n", &x, &y, &z, &r, &g, &b);
                    *p++ = Vec3(x,y,z);
                    *c++ = Vec3(r,g,b);
                }
                else
                {
                    real32 x,y,z;
                    sscanf(line, "v %f %f %f\n", &x, &y, &z);
                    *p++ = Vec3(x,y,z);
                }
            }
            else if(line[0] == 'v' && line[1] == 'n')
            {
                real32 x,y,z;
                sscanf(line, "vn %f %f %f\n", &x, &y, &z);
                *n++ = Vec3(x,y,z);
            }
            else if(line[0] == 'v' && line[1] == 't')
            {
                real32 u,v;
                sscanf(line, "vt %f %f\n", &u, &v);
                *uv++ = Vec2(u,v);
            }
            else if(line[0] == 'f')
            {
                uint32 l = 0,j = 0,k = 0;
                if (nCount == 0)
                {
                    // Read the face without normal and uvs
                    sscanf(line, "f %d %d %d", &l, &j, &k);
                }
                else if(uCount == 0)
                {
                    // Read the face with normals and without uvs
                    uint32 ln,jn,kn;
                    sscanf(line, "f %d//%d %d//%d %d//%d", &l, &ln, &j, &jn, &k, &kn);
                    ASSERT(l==ln && j==jn && k==kn, "Only smooth normals are supported");
                }
                else
                {
                    // Read the face with normals and uvs
                    uint32 ln,jn,kn,lu,ju,ku;
                    sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &l, &lu, &ln, &j, &ju, &jn, &k, &ku, &kn);
                    ASSERT(l==ln && j==jn && k==kn, "Only smooth normals are supported");
                    ASSERT(l==lu && j==ju && k==ku, "Only smooth uvs are supported");
                }
                *i++ = l-1;
                *i++ = j-1;
                *i++ = k-1;
            }
        }
        
        ASSERT(nCount == 0 || vCount == nCount, "Not same numbers of vertices and normals");
        ASSERT(nCount == 0 || n-p == vCount, "");
    
        ASSERT(i == end, "end pointers should match");
        return end;
        
        fclose(file);
        
    }
    else
    {
        ASSERT(false, "couldn't open mesh");
        return memoryPool;
    }
    
}

