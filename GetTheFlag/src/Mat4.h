#ifndef MAT4H
#define MAT4H

#include "GetTheFlag.h"

struct Mat4 {
    real32 data[16];
};

void identity(Mat4& mat)
{
    memset(mat.data, 0, 16*sizeof(real32));
    mat.data[0] = 1.f;
    mat.data[5] = 1.f;
    mat.data[10] = 1.f;
    mat.data[15] = 1.f;
}

void lookAt(Mat4& mat, const Vec3& position, const Vec3& target, const Vec3& up_)
{
    Vec3 forward = normalize(target - position);
    Vec3 right = normalize(cross(forward,up_));
    Vec3 up = cross(right,forward);
    
    
    mat.data[0] = right.x;
    mat.data[4] = right.y;
    mat.data[8] = right.z;
    
    mat.data[1] = up.x;
    mat.data[5] = up.y;
    mat.data[9] = up.z;
    
    mat.data[2] =  -forward.x;
    mat.data[6] =  -forward.y;
    mat.data[10] = -forward.z;
    
    mat.data[3] = position.x;
    mat.data[7] = position.y;
    mat.data[11] = position.z;
    
    mat.data[15] = 1;
}

void inverseTransform(Mat4& m)
{
    // Transpose the 3*3 transformation
    real32 tmp = m.data[4];
    m.data[4] = m.data[1];
    m.data[1] = tmp;
    
    tmp = m.data[8];
    m.data[8] = m.data[2];
    m.data[2] = tmp;
    
    tmp = m.data[9];
    m.data[9] = m.data[6];
    m.data[6] = tmp;
    
    // Negate the translation component and rotate
    real32 tx = -m.data[3];
    real32 ty = -m.data[7];
    real32 tz = -m.data[11];
    
    m.data[3] =  tx*m.data[0] + ty*m.data[1] + tz*m.data[2];
    m.data[7] =  tx*m.data[4] + ty*m.data[5] + tz*m.data[6];
    m.data[11] = tx*m.data[8] + ty*m.data[9] + tz*m.data[10];
}

#endif