//
// Title:	        Spectrum 48K keyboard
// Description:		Spectrum 48K keyboard scanning routines
// Author:	        Dean Belfield
// Created:	        25/05/2026
// Last Updated:	28/05/2026
//
// Modinfo:

#pragma once

#include <cstdint>
#include <SDL2/SDL.h>

#include "ports.h"
#include "defines.h"

using namespace std;

class Keyboard {
public:
    Keyboard(Ports* ports);
    ~Keyboard();

    void   press(SDL_Keycode sym, bool pressed);
private:
    Ports* ports;
    void   port(uint8_t p, uint8_t bit, bool pressed);
};