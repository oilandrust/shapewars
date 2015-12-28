#ifndef INPUTH
#define INPUTH

#include "GetTheFlag.h"
#include <SDL.h>

struct KeyState {
    bool clicked;
    bool held;
};

enum KEY {
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
    DEBUG_SHOW_CONTOURS,
    DEBUG_SHOW_TRI_REGIONS,
    DEBUG_SHOW_POLY_REGIONS,
    DEBUG_SHOW_DUAL_MESH,
    DEBUG_SHOW_PATH,

    NUM_KEYS
};

struct Input {
    KeyState keyStates[NUM_KEYS];

    int32 mouseX;
    int32 mouseY;
};

void initializeInput(Input* input);

void processInput(Input* input);

#endif