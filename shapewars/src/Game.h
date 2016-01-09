#ifndef GAMEH
#define GAMEH

#include "Entities.h"
#include "Mat3.h"
#include "Mat4.h"
#include "ShapeWars.h"
#include "Vec2.h"

struct Level;
struct Renderer;

struct Game {
    Level* level;
    AIEntity bot;
};

void renderGame(Game* game, Renderer* renderer);

#endif