#ifndef INPUTH
#define INPUTH

#include "ShapeWars.h"
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
    MOUSE_LEFT,
    MOUSE_RIGHT,
    QUIT,
    ESCAPE,

    DEBUG_RELOAD_SHADERS,
    DEBUG_TOGGLE_FULLSCREEN,
    DEBUG_SHOW_MENU,
	
    NUM_KEYS
};

struct Input {
    KeyState keyStates[NUM_KEYS];

    int32 mouseX;
    int32 mouseY;
    int32 mousedX;
    int32 mousedY;
};

void initializeInput(Input* input);

void processInput(Input* input);

#endif