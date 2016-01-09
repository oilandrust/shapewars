#ifndef GAMEH
#define GAMEH

#include "Entities.h"
#include "Mat3.h"
#include "Mat4.h"
#include "ShapeWars.h"
#include "Vec2.h"

struct Level;
struct Renderer;
struct NavMesh;
struct Debug;

/* Util structs and functions */
struct ViewCamera {
    Mat4 projection;
    Mat4 view;
    Vec3 position;
    Vec3 forward;
    Vec3 up;
    Vec3 right;
    real32 focalDistance;
    real32 aspect;
};

struct Ray {
    Vec3 origin;
    Vec3 direction;
};

void viewCameraLookAt(ViewCamera* camera, const Vec3& position, const Vec3& target, const Vec3& up);

Vec3 intersectGround0(const Ray& ray);

Ray unproject(ViewCamera* camera, const Vec2& screenPos);

/* Game Here */
struct Game {
    Level* level;
    MemoryArena* tempArena;
    NavMesh* navMesh;
    Debug* debug;

    CameraPan camera;
    ViewCamera viewCamera;
    Vec2 screenSize;

    AIEntity bot;
};

void initializeGame(Game* game);

void handleInputAndUpdateGame(Game* game, Input* input, real32 dt);

void renderGame(Game* game, Renderer* renderer);

#endif