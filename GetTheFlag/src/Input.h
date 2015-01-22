#include <SDL.h>

struct Input {
    bool upKeyDown;
    bool downKeyDown;
    bool leftKeyDown;
    bool rightKeyDown;
    bool escapeKeyDown;
    bool quitKeyDown;
};


void processInput(Input* input);