#include "Game.h"

#include "Level.h"
#include "Renderer.h"

void renderGame(Game* game, Renderer* renderer)
{
    Mat3 identity3;
    identity(identity3);

    Level* level = game->level;

    Vec3 groundSize(level->width, level->height, 0.0f);
    Vec3 groundCenter(0.5f * level->width, 0.5f * level->height, 0.f);

    pushPlanePiece(renderer, &renderer->groundShader, identity3, groundSize, groundCenter, Vec3(.70f));

    // Draw Walls
    Vec3 boxColor = Vec3(0.75);
    Vec3 boxSize = Vec3(1.f);
    Vec3* walls = level->walls;
    uint32 wallCount = level->wallCount;

    for (uint32 i = 0; i < wallCount; i++) {
        pushBoxPiece(renderer, &renderer->flatDiffShader, identity3, boxSize, walls[i], boxColor);
    }

    // Bot box
    Vec3 botPos = game->bot.entity.position + Vec3(0.f, 0.f, .5f);
    pushBoxPiece(renderer, &renderer->flatDiffShader, identity3, Vec3(0.5), botPos, boxColor);
}