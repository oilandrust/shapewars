#include "Entities.h"
#include "Input.h"
#include "Level.h"

struct Node {
    real32 cost;
    real32 estimate;
    int32 pos;
};

struct Heap {
    Node nodes[1024];
    uint32 next = 0;
};

inline uint32 parent(uint32 node) {
    return (node - 1) / 2;
}

inline uint32 left(uint32 node) {
    return 2*node+1;
}

inline uint32 right(uint32 node) {
    return 2*node+2;
}

inline real32 cost(const Node& node) {
    return node.cost+node.estimate;
}

void push(Heap* heap, Node node) {
    Node* nodes = heap->nodes;

    uint32 i = heap->next++;
    nodes[i] = node;
    
    while(i > 0) {
        uint32 p = parent(i);
        if(cost(nodes[i]) < cost(nodes[p])) {
            Node tmp = nodes[i];
            nodes[i] = nodes[p];
            nodes[p] = tmp;
            i = p;
        } else {
            return;
        }
    }
}

Node top(Heap* heap) {
    return heap->nodes[0];
}

bool isEmpty(Heap* heap) {
    return heap->next == 0;
}

void pop(Heap* heap) {
    Node* nodes = heap->nodes;
    nodes[0] = nodes[--heap->next];
    
    uint32 i = 0;
    uint32 length = heap->next;
    
    while(i < heap->next) {
        uint32 m = length;
        
        uint32 l = left(i);
        if(l < length) {
            m = l;
            uint32 r = right(i);
            if(r < length && cost(nodes[r]) < cost(nodes[l])) {
                m = r;
            }
        }
        
        if(m < length && cost(nodes[m]) < cost(nodes[i])) {
            Node tmp = nodes[i];
            nodes[i] = nodes[m];
            nodes[m] = tmp;
            i = m;
        } else {
            return;
        }
    }
}

inline real32 dist(const Vec2i& a, const Vec2i& b) {
    return abs(a.x-b.x) + abs(a.y-b.y);
}

// p = i + j*w
inline Vec2i indexToVec(int32 w, int32 p) {
    int32 i = p % w;
    int32 j = p / w;
    return Vec2i(i, j);
}

inline int32 vecToIndex(int32 w, const Vec2i v) {
    return v.x + w * v.y;
}

void setAIEntityTarget(Level* level, AIEntity* aiEntity, const Vec3& target) {
    aiEntity->target = target;
    
    Vec3 offset = aiEntity->target - aiEntity->entity.position;
    aiEntity->entity.velocity = 0.3*normalize(offset);
    
    int32 startPos = levelCellIndex(level, aiEntity->entity.position);
    int32 endPos = levelCellIndex(level, target);
    
    Vec2i start(aiEntity->entity.position.x, aiEntity->entity.position.y);
    Vec2i end(target.x, target.y);
    
    Heap heap;
    push(&heap, {.0f, dist(start, end), startPos});
    
    int32 width = level->width;
    int32 height = level->height;
    
    int32 pred[1024];
    for(int32 i = 0; i < 1024; i++) {
        pred[i] = -2;
    }
    
    pred[startPos] = -1;
    bool found = false;
    int32 pathLength;
    
    while(!isEmpty(&heap)) {
        Node curr = top(&heap);
        
        if(curr.pos == endPos) {
            found = true;
            pathLength = curr.cost+1;
            break;
        }
        
        Vec2i currPos = indexToVec(width, curr.pos);
        pop(&heap);
        
        Vec2i up(currPos.x, currPos.y+1);
        if(up.y < height && levelValueAt(level, up) == 0) {
            int32 index = vecToIndex(width, up);
            if(pred[index] == -2) {
                pred[index] = curr.pos;
                push(&heap,{curr.cost+1, dist(up, end), index});
            }
        }
        
        Vec2i down(currPos.x, currPos.y-1);
        if(down.y >= 0 && levelValueAt(level, down) == 0) {
            int32 index = vecToIndex(width, down);
            if(pred[index] == -2) {
                pred[index] = curr.pos;
                push(&heap,{curr.cost+1, dist(down, end), index});
            }
        }
        
        Vec2i left(currPos.x-1, currPos.y);
        if(left.x >=0 && levelValueAt(level, left) == 0) {
            int32 index = vecToIndex(width, left);
            if(pred[index] == -2) {
                pred[index] = curr.pos;
                push(&heap,{curr.cost+1, dist(left, end), index});
            }
        }
        
        Vec2i right(currPos.x+1, currPos.y);
        if(right.y < width && levelValueAt(level, right) == 0) {
            int32 index = vecToIndex(width, right);
            if(pred[index] == -2) {
                pred[index] = curr.pos;
                push(&heap,{curr.cost+1, dist(right, end), index});
            }
        }
    }
    aiEntity->pathLength = 0;
    if(found) {
        uint32* path = aiEntity->path;
        aiEntity->pathLength = pathLength;
        
        uint32 i = pathLength-1;
        int32 pos = endPos;
        do {
            path[i--] = pos;
            pos = pred[pos];
        } while (pos != -1);
    }
}

void updateAIEntity(AIEntity* aiEntity) {
    Vec3 offset = aiEntity->target - aiEntity->entity.position;
    if (length(offset) < 0.5) {
        aiEntity->entity.velocity = Vec3(0);
    }
}

void updateEntity(Entity* entity) {
    entity->position += entity->velocity;
}

/*
 * PLAYER
 */

void initializePlayer(Player* player)
{
    // acceleration and drag in m/s;
    player->drag = 20.0f;
    player->acc = 200.0f;
    player->position = Vec3(10, 10, 0);
    player->velocity = Vec3(0, 0, 0);
    player->aimDir = Vec3(1, 0, 0);
    player->size = 3.0f;
    player->collisionSize = Vec2(0.5, 0.5);
}

void updatePlayer(Player* player, Input* input, Level* level, real32 dt)
{
    player->accel = Vec3(0, 0, 0);
    if (input->keyStates[UP].held) {
        player->accel.y = 1.0f;
    }
    if (input->keyStates[DOWN].held) {
        player->accel.y = -1.0f;
    }
    if (input->keyStates[LEFT].held) {
        player->accel.x = -1.0f;
    }
    if (input->keyStates[RIGHT].held) {
        player->accel.x = 1.0f;
    }
    if (std::abs(player->accel.x) > 0.0f || std::abs(player->accel.y) > 0.0f) {
        player->accel = normalize(player->accel);
        player->aimDir = player->accel;
    }
    Vec3 acceleration = player->acc * player->accel - player->drag * player->velocity;
    player->velocity += acceleration * dt;

    if (std::abs(player->velocity.x) > 0.0f || std::abs(player->velocity.y) > 0.0f) {
        Vec3 newPos = player->position + player->velocity * dt;
        Vec2 newPos2D = { newPos.x, newPos.y };
        Vec2 pos2D = { player->position.x, player->position.y };
        Vec2 velocity2D = { player->velocity.x, player->velocity.y };

        // Resolve collisions
        Vec2 collisionPoint;
        Vec2 normal;
        if (levelRectCollides(level,
                Vec2(player->collisionSize), pos2D, velocity2D, dt,
                collisionPoint, normal)) {
            // if we collide, we run the remaining distance along the wall
            real32 dtr;
            do {
                Vec2 dispInside = newPos2D - collisionPoint;
                Vec2 dispCorrected = dispInside - dot(normal, dispInside) * normal;

                if (length(dispCorrected) > 0) {
                    real32 oldVelocity = length(player->velocity);
                    dtr = dt - length(pos2D - collisionPoint) * oldVelocity;
                    velocity2D = oldVelocity * normalize(dispCorrected);
                    pos2D = collisionPoint + dispCorrected;
                }
                else {
                    pos2D = collisionPoint;
                    break;
                }
            } while (dtr > 0 && levelRectCollides(level,
                                    player->collisionSize, pos2D, velocity2D, dtr,
                                    collisionPoint, normal));

            if (levelRectCollides(level,
                    player->collisionSize, pos2D, velocity2D, dt,
                    collisionPoint, normal)) {
                pos2D = collisionPoint;
            }

            player->position = { pos2D, 0 };
            player->velocity = { velocity2D, 0 };
        }
        else {
            player->position = newPos;
        }

        // Stay inside the Level
        Vec3 size3 = Vec3(player->collisionSize.x, player->collisionSize.y, 0);
        player->position = max(player->position, 0.5f * size3);
        player->position = min(player->position, Vec3(level->width, level->height, 0) - 0.5f * size3);
    }
}

/**
 * Camera
 */

void initializeCameraPan(CameraPan* camera)
{
    Vec3 targetToCamOffset(.0f, -0.9 * 30, 20);
    camera->position = camera->target + targetToCamOffset;

    camera->velocity = { 0, 0 };
    camera->accel = { 0, 0 };
    camera->drag = 10.0f;
    
    // acceleration and drag in m/s;
    camera->drag = 20.0f;
    camera->acc = 200.0f;
}

void updateCameraPan(CameraPan* camera, Input* input, Level* level, real32 dt)
{
    Vec3 accel = Vec3(0, 0, 0);
    if (input->keyStates[UP].held) {
        accel.y = 1.0f;
    }
    if (input->keyStates[DOWN].held) {
        accel.y = -1.0f;
    }
    if (input->keyStates[LEFT].held) {
        accel.x = -1.0f;
    }
    if (input->keyStates[RIGHT].held) {
        accel.x = 1.0f;
    }
    if (std::abs(accel.x) > 0.0f || std::abs(accel.y) > 0.0f) {
        accel = normalize(accel);
    }
    
    Vec3 acceleration = camera->acc * accel - camera->drag * camera->velocity;
    camera->velocity += acceleration * dt;
    
    camera->target = camera->target + camera->velocity * dt;
    camera->target = min(camera->target, Vec3(level->width, level->height, 0));

    Vec3 targetToCamOffset(.0f, -10, 30);
    camera->position = camera->target + targetToCamOffset;

}
