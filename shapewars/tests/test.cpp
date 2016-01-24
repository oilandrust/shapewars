#include <SDL.h>

#include "NavMeshGen.h"

void scase1(MemoryArena* arena) {
	const uint32 w = 3;
	const uint32 h = 5;
	int32 ids[w * h] = { -1, -1, -1,
						-1,	0, -1,
						-1,	0, -1,
						-1,	0, -1,
						-1, -1, -1 };
	Contour contour;

	walkCountour(arena, &contour, ids, w, h, 0,0,0,0, 1, 1);
	ASSERT(contour.count == 4);
	
	resetArena(arena);
}

void scase2(MemoryArena* arena) {
	const uint32 w = 4;
	const uint32 h = 4;
	int32 ids[w * h] = { -1, -1, -1, -1,
						 -1, -1, 0, -1,
						 -1,  0, 0, -1,
						 -1, -1, -1, -1 };
	Contour contour;

	walkCountour(arena, &contour, ids, w, h,  0, 0, 0, 0, 2, 1);
	ASSERT(contour.count == 6);

	resetArena(arena);
}

void scase3(MemoryArena* arena) {
	const uint32 w = 3;
	const uint32 h = 3;
	int32 ids[w * h] = { -1, -1, -1,
						-1, 0, -1,
						-1, -1, -1 };
	Contour contour;

	walkCountour(arena, &contour, ids, w, h, 0, 0, 0, 0, 2, 1);
	ASSERT(contour.count == 4);

	resetArena(arena);
}

void scase4(MemoryArena* arena) {
	const uint32 w = 4;
	const uint32 h = 3;
	int32 ids[w * h] = { -1, -1, -1, -1,
						-1, 0, 0, -1,
						-1, -1, -1, -1 };
	Contour contour;

	walkCountour(arena, &contour, ids, w, h, 0, 0, 0, 0, 1, 1);
	ASSERT(contour.count == 4);

	resetArena(arena);
}

void case1(MemoryArena* arena) {
	const uint32 w = 5;
	const uint32 h = 3;
	int32 ids[w * h] = { 1, 1, 2, 1, 1,
						 1, 0, 0, 0, 1,
						 1, 2, 1, 1, 2 };
	Contour contour;

	walkCountour(arena, &contour, ids, w, h, 0, 0, 0, 0, 1, 1);
	ASSERT(contour.count == 7);

	resetArena(arena);
}

void case2(MemoryArena* arena) {
	const uint32 w = 9;
	const uint32 h = 6;
	int32 ids[w * h] = { 1, 1, 1, 1, 1, 1, 1, 1, 1,
						1, 3, 1, 2, 0, 1, 1, 1, 1,
						1, 1, 0, 0, 0, 1, 0, 1, 1,
						1, 3, 0, 0, 0, 0, 0, 2, 1,
						1, 1, 2, 0, 0, 1, 1, 1, 1,
						1, 1, 2, 1, 1, 1, 1, 1, 1 };

	Contour contour;

	walkCountour(arena, &contour, ids, w, h, 0, 0, 0, 0, 4, 1);
	ASSERT(contour.count == 17);

	resetArena(arena);
}


int main(int argc, char *argv[])
{
	MemoryArena arena;
	initializeArena(&arena, new uint8[Megabytes(1)], Megabytes(1));
	
	// Simple cases without id change.
	scase1(&arena);
	scase2(&arena);
	scase3(&arena);
	scase4(&arena);

	case1(&arena);
	case2(&arena);

	/*
	{ x = 13.5000000 y = 27.5000000 z = 0.500000000 }	Vec3
	{ x = 19.5000000 y = 27.5000000 z = 0.500000000 }	Vec3
	{ x = 19.5000000 y = 29.5000000 z = 0.500000000 }	Vec3
	{ x = 20.0000000 y = 29.5000000 z = 0.500000000 }	Vec3
	{ x = 20.0000000 y = 30.0000000 z = 0.500000000 }	Vec3
	{ x = 20.5000000 y = 30.0000000 z = 0.500000000 }	Vec3
	{ x = 20.5000000 y = 30.5000000 z = 0.500000000 }	Vec3
	{ x = 23.5000000 y = 30.5000000 z = 0.500000000 }	Vec3
	{ x = 23.5000000 y = 32.0000000 z = 0.500000000 }	Vec3
	{ x = 13.5000000 y = 32.0000000 z = 0.500000000 }	Vec3
	*/

	return 0;
}