#ifndef INPUTH
#define INPUTH

#include <SDL.h>
#include "GetTheFlag.h"

struct KeyState {
    bool clicked;
    bool held;
};

enum KEYS {
    UP = 0,
    DOWN,
    LEFT,
    RIGHT,
    FIRE1,
    FIRE2,
    QUIT,
    ESCAPE,

    DEBUG_RELOAD_SHADERS,
    DEBUG_SHOW_DISTANCE_FIELD,
    DEBUG_SHOW_REGIONS,
    
    NUM_KEYS
};

struct Input {
    KeyState keyStates[NUM_KEYS];
    
    int32 mouseX;
    int32 mouseY;
};


void processInput(Input* input);

#endif