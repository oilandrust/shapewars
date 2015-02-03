#include "Entities.h"
#include "Input.h"
#include "Level.h"


void initializeBullets(BulletManager* bulletManager)
{
    bulletManager->bulletCount = 0;
    bulletManager->bulletSpeed = 30.0f;
    bulletManager->bulletSize = Vec2(0.43f);
    
    loadTexture(&bulletManager->bulletTexture, "data/bullet.png");
}
uint32 createBullet(BulletManager* manager)
{
    ASSERT(manager->bulletCount < MAX_BULLET_COUNT - 1, "Too many bullets");
    return manager->bulletCount++;
}

void releaseBullet(BulletManager* manager, uint32 bulletIndex)
{
    manager->bullets[bulletIndex] = manager->bullets[--manager->bulletCount];
}

void updateBullets(BulletManager* manager, Level* level, real32 dt)
{
    // Move the bullet forward
    real32 levelWidth = (real32)level->width;
    real32 levelHeight = (real32)level->height;
    Rect bulletRect;
    
    uint32 bulletCount = manager->bulletCount;
    Bullet* bullets = manager->bullets;
    real32 bulletSpeed = manager->bulletSpeed;
    Vec2 bulletSize = manager->bulletSize;
    
    for (uint32 i = 0; i < bulletCount; i++)
    {
        bullets[i].position += (bulletSpeed * dt) * bullets[i].velocity;
        
        bulletRect.min = bullets[i].position - 0.5f*bulletSize;
        bulletRect.max = bullets[i].position + 0.5f*bulletSize;
        
        if (isOutsideLevel(levelWidth, levelHeight, bullets[i].position)
            || levelRectCollides(level, bulletRect))
        {
            releaseBullet(manager, i);
        }
    }
}

/*
 * PLAYER
 */

void initializePlayer(Player* player)
{
    // acceleration and drag in m/s;
    player->drag = 20.0f;
    player->acc = 200.0f;
    player->position = Vec2(3,3);
    player->velocity = Vec2(0,0);
    player->aimDir = Vec2(1,0);
    player->size = 3.0f;
    player->collisionSize = Vec2(0.4*player->size, 0.4*player->size);
}

void updatePlayer(Player* player, Input* input, Level* level, real32 dt)
{
    player->accel = Vec2(0,0);
    if (input->keyStates[UP].held)
    {
        player->accel.y = 1.0f;
    }
    if (input->keyStates[DOWN].held)
    {
        player->accel.y = -1.0f;
    }
    if (input->keyStates[LEFT].held)
    {
        player->accel.x = -1.0f;
    }
    if (input->keyStates[RIGHT].held)
    {
        player->accel.x = 1.0f;
    }
    if(abs(player->accel.x) > 0.0f || abs(player->accel.y) > 0.0f)
    {
        
        player->accel = normalize(player->accel);
        player->aimDir = player->accel;
    }
    Vec2 acceleration = player->acc*player->accel - player->drag*player->velocity;
    player->velocity += acceleration * dt;
    
    if( abs(player->velocity.x) > 0.0f || abs(player->velocity.y) > 0.0f)
    {
        Vec2 newPos = player->position + player->velocity * dt;;
        
        // Resolve collisions
        Vec2 collisionPoint;
        Vec2 normal;
        if( levelRectCollides(level,
                              player->collisionSize, player->position, player->velocity, dt,
                              collisionPoint,normal ) )
        {
            // if we collide, we run the remaining distance along the wall
            real32 dtr;
            do
            {
                Vec2 dispInside = newPos - collisionPoint;
                Vec2 dispCorrected = dispInside-dot(normal,dispInside)*normal;
                
                if(length(dispCorrected) > 0)
                {
                    real32 oldVelocity = length(player->velocity);
                    dtr = dt - length(player->position - collisionPoint)*oldVelocity;
                    player->velocity = oldVelocity * normalize(dispCorrected);
                    player->position = collisionPoint + dispCorrected;
                }
                else
                {
                    player->position = collisionPoint;
                    break;
                }
            }
            while(dtr > 0 && levelRectCollides(level,
                                               player->collisionSize, player->position, player->velocity, dtr,
                                               collisionPoint,normal ));
            
            if(levelRectCollides(level,
                                 player->collisionSize, player->position, player->velocity, dt,
                                 collisionPoint,normal ))
            {
                player->position = collisionPoint;
            }
        }
        else
        {
            player->position = newPos;
        }
        
        // Stay inside the Level
        player->position = max(player->position,0.5f*player->collisionSize);
        player->position = min(player->position,Vec2(level->width,level->height)-0.5f*player->collisionSize);
    }

}
