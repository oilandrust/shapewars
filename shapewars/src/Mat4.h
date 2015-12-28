#ifndef MAT4H
#define MAT4H

#include "ShapeWars.h"

#include "Vec3.h"

struct Mat4 {
    real32 data[16];
};

inline void identity(Mat4& mat)
{
    memset(mat.data, 0, 16 * sizeof(real32));
    mat.data[0] = 1.f;
    mat.data[5] = 1.f;
    mat.data[10] = 1.f;
    mat.data[15] = 1.f;
}

void ortho(Mat4& mat, real32 left, real32 right, real32 bottom, real32 top, real32 near, real32 far);

void perspective(Mat4& mat, real32 fovy, real32 aspect, real32 near, real32 far);

void lookAt(Mat4& mat, const Vec3& position, const Vec3& target, const Vec3& up_);

void inverseTransform(Mat4& m);

#endif