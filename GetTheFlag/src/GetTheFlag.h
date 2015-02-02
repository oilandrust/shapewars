#ifndef GETTHEFLAGH
#define GETTHEFLAGH

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <cassert>
#include <csignal>

#ifdef _MSC_VER
    #define DEBUG_BREAK __debugbreak()
#else
    #define DEBUG_BREAK raise(SIGTRAP)
#endif

#ifndef NDEBUG
    #   define ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
            << " line " << __LINE__ << ": " << message << std::endl; \
            DEBUG_BREAK; \
        } \
    } while (false)
#else
    #   define ASSERT(condition, message) do { } while (false)
#endif

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

#define PI 3.14159265

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