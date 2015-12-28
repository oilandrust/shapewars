#ifndef MAT3H
#define MAT3H

#include "GetTheFlag.h"

struct Mat3 {
    real32 data[9];
};

inline void identity(Mat3& mat)
{
    memset(mat.data, 0, 9 * sizeof(real32));
    mat.data[0] = 1.f;
    mat.data[4] = 1.f;
    mat.data[8] = 1.f;
}

inline void rotationZ(Mat3& mat, real32 angle)
{
    real32 c = cos(angle);
    real32 s = sin(angle);
    mat.data[0] = c;
    mat.data[1] = -s;
    mat.data[2] = 0.f;
    mat.data[3] = s;
    mat.data[4] = c;
    mat.data[5] = 0.f;
    mat.data[6] = 0.f;
    mat.data[7] = 0.f;
    mat.data[8] = 1.f;
}

#endif