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

// Pre-shifted column masks for keys
//
#define COL0 0b00000001
#define COL1 0b00000010
#define COL2 0b00000100
#define COL3 0b00001000
#define COL4 0b00010000

using namespace std;

// Represents a single Spectrum key
//
struct Key {
	uint8_t port;	// The Z80 port the key is on
	uint8_t bit;	// The bit (column) in that port
};

class Keyboard {
public:
    Keyboard(Ports* ports);

    void   press(SDL_Keycode sym, bool pressed);
private:
    Ports* ports;

	// Map of SDL2 keypresses to Spectrum port and columns
	//
	map<SDL_Keycode, Key> keys = {
        { SDLK_LSHIFT,	{ 0xFE, COL0 } },
        { SDLK_z,		{ 0xFE, COL1 } },
        { SDLK_x,		{ 0xFE, COL2 } },
        { SDLK_c,		{ 0xFE, COL3 } },
        { SDLK_v,		{ 0xFE, COL4 } },
        { SDLK_a,		{ 0xFD, COL0 } },
        { SDLK_s,		{ 0xFD, COL1 } },
        { SDLK_d,		{ 0xFD, COL2 } },
        { SDLK_f,		{ 0xFD, COL3 } },
        { SDLK_g,		{ 0xFD, COL4 } },
        { SDLK_q,		{ 0xFB, COL0 } },
        { SDLK_w,		{ 0xFB, COL1 } },
        { SDLK_e,		{ 0xFB, COL2 } },
        { SDLK_r,		{ 0xFB, COL3 } },
        { SDLK_t,		{ 0xFB, COL4 } },
        { SDLK_1,		{ 0xF7, COL0 } },
        { SDLK_2,		{ 0xF7, COL1 } },
        { SDLK_3,		{ 0xF7, COL2 } },
        { SDLK_4,		{ 0xF7, COL3 } },
        { SDLK_5,		{ 0xF7, COL4 } },
        { SDLK_0,		{ 0xEF, COL0 } },
        { SDLK_9,		{ 0xEF, COL1 } },
        { SDLK_8,		{ 0xEF, COL2 } },
        { SDLK_7,		{ 0xEF, COL3 } },
        { SDLK_6,		{ 0xEF, COL4 } },
        { SDLK_p,		{ 0xDF, COL0 } },
        { SDLK_o,		{ 0xDF, COL1 } },
        { SDLK_i,		{ 0xDF, COL2 } },
        { SDLK_u,		{ 0xDF, COL3 } },
        { SDLK_y,		{ 0xDF, COL4 } },
        { SDLK_RETURN,	{ 0xBF, COL0 } },
        { SDLK_l,		{ 0xBF, COL1 } },
        { SDLK_k,		{ 0xBF, COL2 } },
        { SDLK_j,		{ 0xBF, COL3 } },
        { SDLK_h,		{ 0xBF, COL4 } },
        { SDLK_SPACE,	{ 0x7F, COL0 } },
        { SDLK_RSHIFT,	{ 0x7F, COL1 } },
        { SDLK_m,		{ 0x7F, COL2 } },
        { SDLK_n,		{ 0x7F, COL3 } },
        { SDLK_b,		{ 0x7F, COL4 } },
	};
};