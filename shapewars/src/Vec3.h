#ifndef VEC3H
#define VEC3H

#include "ShapeWars.h"
#include "Vec2.h"

struct Vec3 {
    real32 x;
    real32 y;
    real32 z;

    Vec3(real32 x_, real32 y_, real32 z_)
        : x(x_)
        , y(y_)
        , z(z_)
    {
    }

    Vec3(real32 x_)
        : x(x_)
        , y(x_)
        , z(x_)
    {
    }

    Vec3(const Vec2& xy, real32 z_)
        : x(xy.x)
        , y(xy.y)
        , z(z_)
    {
    }

    Vec3() {}

	Vec2 xy() const {
		return Vec2(x,y);
	}
};

inline Vec3 operator+(const Vec3& a, const Vec3& b)
{
    return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline Vec3& operator+=(Vec3& a, const Vec3& b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

inline Vec3 operator-(const Vec3& a, const Vec3& b)
{
    return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline Vec3& operator-=(Vec3& a, const Vec3& b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

inline Vec3 operator-(const Vec3& a)
{
    return Vec3(-a.x, -a.y, -a.z);
}

inline Vec3 operator*(const Vec3& a, const Vec3& b)
{
    return Vec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

inline Vec3& operator*=(Vec3& a, const real32& b)
{
    a.x *= b;
    a.y *= b;
    a.z *= b;
    return a;
}

inline Vec3 operator*(const Vec3& a, real32 l)
{
    return Vec3(l * a.x, l * a.y, l * a.z);
}

inline Vec3 operator*(const real32& f, const Vec3& v)
{
    return Vec3(f * v.x, f * v.y, f * v.z);
}

inline real32 dot(const Vec3& v1, const Vec3& v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline real32 sqrLength(const Vec3& v)
{
    return dot(v, v);
}

inline real32 length(const Vec3& v)
{
    return sqrtf(dot(v, v));
}

inline Vec3 normalize(const Vec3& v)
{
    ASSERT_MSG(dot(v, v) > 0, "Normalize zero vector!");
    real32 linv = 1.0f / length(v);
    return Vec3(v.x * linv, v.y * linv, v.z * linv);
}

inline Vec3 max3(const Vec3& v1, const Vec3& v2)
{
    return Vec3(max(v1.x, v2.x), max(v1.y, v2.y), max(v1.z, v2.z));
}

inline Vec3 min3(const Vec3& v1, const Vec3& v2)
{
    return Vec3(min(v1.x, v2.x), min(v1.y, v2.y), min(v1.z, v2.z));
}

inline Vec3 cross(const Vec3& a, const Vec3& b)
{
    return Vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x);
}

#endif