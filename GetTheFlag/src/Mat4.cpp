#include "Mat4.h"

void ortho(Mat4& mat, real32 left, real32 right, real32 bottom, real32 top, real32 near, real32 far)
{
    real32 rli = 1.f / (right - left);
    real32 tbi = 1.f / (top - bottom);
    real32 fni = 1.f / (far - near);

    // diag
    mat.data[0] = 2.f * rli;
    mat.data[5] = 2.f * tbi;
    mat.data[10] = -2.f * fni;
    mat.data[15] = 1;

    // trans
    mat.data[3] = -rli * (right + left);
    mat.data[7] = -tbi * (top + bottom);
    mat.data[11] = -fni * (far + near);
}

void perspective(Mat4& mat, real32 fovy, real32 aspect, real32 near, real32 far)
{
    real32 f = 1.f / tanf(.5f * fovy * PI / 180.f);
    real32 iNearFar = 1.f / (far - near);

    mat.data[0] = f / aspect;
    mat.data[5] = f;
    mat.data[10] = -iNearFar * (far + near);
    mat.data[11] = -2.f * far * near * iNearFar;
    mat.data[14] = -1.f;
}

void lookAt(Mat4& mat, const Vec3& position, const Vec3& target, const Vec3& up_)
{
    memset(mat.data, 0, 16 * sizeof(real32));

    Vec3 forward = normalize(target - position);
    Vec3 right = normalize(cross(forward, up_));
    Vec3 up = cross(right, forward);

    mat.data[0] = right.x;
    mat.data[4] = right.y;
    mat.data[8] = right.z;

    mat.data[1] = up.x;
    mat.data[5] = up.y;
    mat.data[9] = up.z;

    mat.data[2] = -forward.x;
    mat.data[6] = -forward.y;
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

    m.data[3] = tx * m.data[0] + ty * m.data[1] + tz * m.data[2];
    m.data[7] = tx * m.data[4] + ty * m.data[5] + tz * m.data[6];
    m.data[11] = tx * m.data[8] + ty * m.data[9] + tz * m.data[10];
}
