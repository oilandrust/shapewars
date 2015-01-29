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
    RELOAD,
    
    NUM_KEYS
};

struct Input {
    KeyState keyStates[NUM_KEYS];
};


void processInput(Input* input);

#endif