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
#include <map>
#include <SDL2/SDL.h>

#include "ports.h"
#include "defines.h"

using namespace std;

// Represents a single Spectrum key
//
struct Key {
	uint8_t port;	// The Z80 port the key is on
	uint8_t column;	// The column (bit) in that port
};

class Keyboard {
public:
    Keyboard(Ports* ports);

    void   press(SDL_Keycode sym, bool pressed);
private:
    Ports* ports;
    void   port(uint8_t p, uint8_t bit, bool pressed);

	// Map of SDL2 keypresses to Spectrum port and columns
	//
	map<SDL_Keycode, Key> keys = {
        { SDLK_LSHIFT,	{ 0xFE, 0 } },
        { SDLK_z,		{ 0xFE, 1 } },
        { SDLK_x,		{ 0xFE, 2 } },
        { SDLK_c,		{ 0xFE, 3 } },
        { SDLK_v,		{ 0xFE, 4 } },
        { SDLK_a,		{ 0xFD, 0 } },
        { SDLK_s,		{ 0xFD, 1 } },
        { SDLK_d,		{ 0xFD, 2 } },
        { SDLK_f,		{ 0xFD, 3 } },
        { SDLK_g,		{ 0xFD, 4 } },
        { SDLK_q,		{ 0xFB, 0 } },
        { SDLK_w,		{ 0xFB, 1 } },
        { SDLK_e,		{ 0xFB, 2 } },
        { SDLK_r,		{ 0xFB, 3 } },
        { SDLK_t,		{ 0xFB, 4 } },
        { SDLK_1,		{ 0xF7, 0 } },
        { SDLK_2,		{ 0xF7, 1 } },
        { SDLK_3,		{ 0xF7, 2 } },
        { SDLK_4,		{ 0xF7, 3 } },
        { SDLK_5,		{ 0xF7, 4 } },
        { SDLK_0,		{ 0xEF, 0 } },
        { SDLK_9,		{ 0xEF, 1 } },
        { SDLK_8,		{ 0xEF, 2 } },
        { SDLK_7,		{ 0xEF, 3 } },
        { SDLK_6,		{ 0xEF, 4 } },
        { SDLK_p,		{ 0xDF, 0 } },
        { SDLK_o,		{ 0xDF, 1 } },
        { SDLK_i,		{ 0xDF, 2 } },
        { SDLK_u,		{ 0xDF, 3 } },
        { SDLK_y,		{ 0xDF, 4 } },
        { SDLK_RETURN,	{ 0xBF, 0 } },
        { SDLK_l,		{ 0xBF, 1 } },
        { SDLK_k,		{ 0xBF, 2 } },
        { SDLK_j,		{ 0xBF, 3 } },
        { SDLK_h,		{ 0xBF, 4 } },
        { SDLK_SPACE,	{ 0x7F, 0 } },
        { SDLK_RSHIFT,	{ 0x7F, 1 } },
        { SDLK_m,		{ 0x7F, 2 } },
        { SDLK_n,		{ 0x7F, 3 } },
        { SDLK_b,		{ 0x7F, 4 } },
	};
};