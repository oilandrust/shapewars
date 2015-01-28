#ifndef VEC2H
#define VEC2H

#include "GetTheFlag.h"

struct Vec2 {
    real32 x;
    real32 y;
    
    Vec2(real32 x_, real32 y_):x(x_),y(y_){}
    Vec2(){}
};

inline Vec2 operator+(const Vec2& a, const Vec2& b)
{
    return Vec2{a.x+b.x,a.y+b.y};
}

inline Vec2& operator +=(Vec2& a, const Vec2& b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}

inline Vec2 operator-(const Vec2& a, const Vec2& b)
{
    return Vec2{a.x-b.x,a.y-b.y};
}

inline Vec2& operator -=(Vec2& a, const Vec2& b)
{
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

inline Vec2 operator-(const Vec2& a)
{
    return Vec2{-a.x,-a.y};
}

inline Vec2& operator *=(Vec2& a, const real32& b)
{
    a.x *= b;
    a.y *= b;
    return a;
}

inline Vec2 operator*(const Vec2& a, real32 l)
{
    return Vec2{l*a.x,l*a.y};
}

inline Vec2 operator*(const real32& f, const Vec2& v)
{
    return Vec2(f*v.x,f*v.y);
}

inline real32 dot(const Vec2& v1, const Vec2& v2)
{
    return v1.x*v2.x + v1.y*v2.y;
}

inline real32 length(const Vec2& v)
{
    return sqrtf(dot(v,v));
}

inline Vec2 normalize(const Vec2& v)
{
    ASSERT(dot(v, v) > 0,"Normalize zero vector!");
    real32 linv = 1.0f/length(v);
    return Vec2(v.x*linv,v.y*linv);
}

inline Vec2 max(const Vec2& v1, const Vec2& v2)
{
    return Vec2(max(v1.x,v2.x),max(v1.y,v2.y));
}

inline Vec2 min(const Vec2& v1, const Vec2& v2)
{
    return Vec2(min(v1.x,v2.x),min(v1.y,v2.y));
}

#endif