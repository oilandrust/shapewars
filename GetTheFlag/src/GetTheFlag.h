#ifndef GETTHEFLAGH
#define GETTHEFLAGH

#include <cassert>
#include <cmath>

#define ASSERT assert

typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32 ;
typedef long int64 ;
typedef unsigned long uint64;

typedef float real32;
typedef double real64;

template<typename T>
inline T max(const T& a, const T&b)
{
    return a<b?b:a;
}

template<typename T>
inline T min(const T& a, const T&b)
{
    return a<b?a:b;
}

inline uint32 roundReal32toInt32(real32 r)
{
    return (uint32)floor(r+0.5f);
}


#endif