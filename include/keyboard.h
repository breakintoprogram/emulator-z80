#pragma once

#include <cstdint>
#include <SDL2/SDL.h>

#include "defines.h"

using namespace std;

class keyboard {
public:
    keyboard(uint8_t* ports);
    ~keyboard();

    void press(SDL_Keycode sym, bool pressed);
private:
    uint8_t* ports;
    void port(uint8_t p, uint8_t bit, bool pressed);
};