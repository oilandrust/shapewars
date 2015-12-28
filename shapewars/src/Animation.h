#ifndef ANIMATION_H
#define ANIMATION_H

#include <cmath>

#include "GetTheFlag.h"
#include "Vec3.h"

#define MAX_BONES 16
#define MAX_FRAMES 16

// Transforms are relative to parent bone
struct KeyFrame {
    real32 t;
    Vec3 transforms[MAX_BONES];
};

struct Animation {
    real32 length;
    bool playing;
    bool loop;
    real32 t;

    uint32 boneCount;
    Vec3 restTransforms[MAX_BONES];
    int32 parents[MAX_BONES];
    
    // The current frame transforms
    Vec3 transforms[MAX_BONES];
    Vec3 worldTransforms[MAX_BONES];
    
    uint32 keyCount;
    KeyFrame keyFrames[MAX_FRAMES];
};

void animate(Animation *animation, const real32 dt)
{
    if(animation->playing)
    {
        animation->t = fmod(animation->t+dt, animation->length);
        real32 t = animation->t;
        
        uint32 curFrameIndex = 0;
        while (t > animation->keyFrames[curFrameIndex].t && curFrameIndex < animation->keyCount - 1)
        {
            curFrameIndex++;
        }
        curFrameIndex--;
        ASSERT_MSG(curFrameIndex < animation->keyCount, "");
        
        Vec3* key0Trans = animation->keyFrames[curFrameIndex].transforms;
        Vec3* key1Trans = animation->keyFrames[curFrameIndex+1].transforms;
        Vec3* transforms = animation->transforms;
        
        real32 relT = t - animation->keyFrames[curFrameIndex].t;
        for (uint32 i = 0; i < animation->boneCount; i++)
        {
            transforms[i] = (1 - relT) * key0Trans[i] + relT * key1Trans[i];
        }
    }
}

void updateWorldTransforms(Animation* animation)
{
    Vec3* worldTransforms = animation->worldTransforms;
    Vec3* transforms = animation->transforms;
    int32* parents = animation->parents;
    uint32 boneCount = animation->boneCount;
    
    for (uint32 i = 0; i < boneCount; i++)
    {
        if (parents[i] != -1)
        {
            worldTransforms[i] = worldTransforms[parents[i]] + transforms[i];
        }
        else
        {
            worldTransforms[i] = transforms[i];
        }
    }
}

void start(Animation* animation)
{
    animation->playing = true;
    animation->t = 0.f;
}

void stop(Animation* animation)
{
    animation->playing = false;
    animation->t = 0.f;
}


#endif