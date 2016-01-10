#ifndef GETTHEFLAGH
#define GETTHEFLAGH

#include <cassert>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#ifdef _MSC_VER
#define DEBUG_BREAK __debugbreak()
#else
#define DEBUG_BREAK raise(SIGTRAP)
#endif

#ifndef NDEBUG
#define ASSERT_MSG(condition, message, ...)                                                                   \
    do {                                                                                                      \
        if (!(condition)) {                                                                                   \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ << " line " << __LINE__ << ": "; \
            printf(message, ##__VA_ARGS__);                                                                   \
            printf("\n");                                                                                     \
            DEBUG_BREAK;                                                                                      \
        }                                                                                                     \
    } while (false)
#else
#define ASSERT_MSG(condition, message, ...) \
    do {                                    \
    } while (false)
#endif

#define ASSERT assert

#ifdef NDEBUG
#define LOG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define LOG(fmt, ...)
#endif

#define arrayCount(a) (sizeof(a) / sizeof((a)[0]))

typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef long int64;
typedef unsigned long uint64;
typedef float real32;
typedef double real64;

struct RGB {
    uint8 r;
    uint8 g;
    uint8 b;
};

struct RGBA {
    uint8 r;
    uint8 g;
    uint8 b;
    uint8 a;
};

#define PI 3.14159265

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)

template <typename T>
inline T max(const T& a, const T& b)
{
    return a < b ? b : a;
}

template <typename T>
inline T min(const T& a, const T& b)
{
    return a < b ? a : b;
}

inline uint32 roundReal32toInt32(real32 r)
{
    return (uint32)floor(r + 0.5f);
}

inline real32 randRangeReal32(real32 min, real32 max)
{
    real32 range = max - min;
    return min + range * rand() / RAND_MAX;
}

struct MemoryArena {
    size_t used;
    size_t size;
    uint8* begin;
};

struct Memory {
    MemoryArena persistentArena;
    MemoryArena temporaryArena;
};

inline void initializeArena(MemoryArena* arena, void* base, size_t size)
{
    arena->begin = (uint8*)base;
    arena->size = size;
    arena->used = 0;
    for (size_t i = 0; i < size / 4; i++) {
        ((int32*)base)[i] = 0xdeadbeef;
    }
}

inline void resetArena(MemoryArena* arena)
{
    arena->used = 0;
}

inline void* pushSize(MemoryArena* arena, size_t size)
{
    ASSERT(arena->used + size < arena->size);
    void* ptr = (void*)(arena->begin + arena->used);
    arena->used += size;
    return ptr;
}

inline void popSize(MemoryArena* arena, size_t size)
{
    ASSERT(arena->used - size >= 0);
    arena->used -= size;
}

template <class T>
inline T* pushStruct(MemoryArena* arena)
{
    return (T*)pushSize(arena, sizeof(T));
}

template <class T>
inline T* pushStructZeroed(MemoryArena* arena)
{
    T* ptr = (T*)pushSize(arena, sizeof(T));
    memset(ptr, 0, sizeof(T));
    return ptr;
}

template <class T>
inline T* pushArray(MemoryArena* arena, size_t count)
{
    return (T*)pushSize(arena, count * sizeof(T));
}

template <class T>
inline T* pushArrayZeroed(MemoryArena* arena, size_t count)
{
    T* ptr = (T*)pushSize(arena, count * sizeof(T));
    memset(ptr, 0, sizeof(T) * count);
    return ptr;
}

template <class T>
inline T* pushArray(MemoryArena* arena, size_t count, T val)
{
    T* ptr = (T*)pushSize(arena, count * sizeof(T));
    for (uint32 i = 0; i < count; i++) {
        ptr[i] = val;
    }
    return ptr;
}

template <class T>
inline void popArray(MemoryArena* arena, size_t count)
{
    popSize(arena, count * sizeof(T));
}

#endif