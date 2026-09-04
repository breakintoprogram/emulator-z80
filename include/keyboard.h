//
// Title:	        Spectrum 48K keyboard
// Description:		Spectrum 48K keyboard scanning routines
// Author:	        Dean Belfield
// Created:	        25/05/2026
// Last Updated:	04/09/2026
//
// Modinfo:
// 03/09/2026:		Modified key mappings for Windows PCs
// 04/09/2025:		Added ZX_Keycodes

#pragma once

#include <iostream>
#include <cstdint>
#include <map>
#include <vector>
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

// Physical ZX Spectrum keycodes
//
typedef enum {
	ZXK_0, ZXK_1, ZXK_2, ZXK_3, ZXK_4, ZXK_5, ZXK_6, ZXK_7, ZXK_8, ZXK_9,
	ZXK_a, ZXK_b, ZXK_c, ZXK_d, ZXK_e, ZXK_f, ZXK_g, ZXK_h, ZXK_i, ZXK_j,
	ZXK_k, ZXK_l, ZXK_m, ZXK_n, ZXK_o, ZXK_p, ZXK_q, ZXK_r, ZXK_s, ZXK_t,
	ZXK_u, ZXK_v, ZXK_w, ZXK_x, ZXK_y, ZXK_z, ZXK_CAPS, ZXK_SYMS, ZXK_SPACE, ZXK_ENTER
} ZX_Keycode;

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
	
	// Map an SDL keycode to one or more ZX keycodes
	//
	map<SDL_Keycode, vector<ZX_Keycode>> keycodes = {
		//
		// First the enhanced keys
		//
		{ SDLK_BACKSPACE,	{ ZXK_CAPS, ZXK_0 } },
		{ SDLK_LEFT,		{ ZXK_CAPS, ZXK_5 } },
		{ SDLK_DOWN, 		{ ZXK_CAPS, ZXK_6 } },
		{ SDLK_UP,			{ ZXK_CAPS, ZXK_7 } },
		{ SDLK_RIGHT,		{ ZXK_CAPS, ZXK_8 } },
		{ SDLK_COMMA,		{ ZXK_SYMS, ZXK_n } },
		{ SDLK_PERIOD, 		{ ZXK_SYMS, ZXK_m } },
		{ SDLK_SEMICOLON, 	{ ZXK_SYMS, ZXK_o } },
		{ SDLK_QUOTE,		{ ZXK_SYMS, ZXK_7 } },
		{ SDLK_SLASH,		{ ZXK_SYMS, ZXK_v } },
		{ SDLK_MINUS,		{ ZXK_SYMS, ZXK_j } },
		{ SDLK_EQUALS,		{ ZXK_SYMS, ZXK_l } },
		{ SDLK_ESCAPE,		{ ZXK_CAPS, ZXK_SPACE } },
		//
		// Then the standard keys
		//
		{ SDLK_LSHIFT,		{ ZXK_CAPS } },
		{ SDLK_RSHIFT,		{ ZXK_CAPS } },
		{ SDLK_z,			{ ZXK_z } },
		{ SDLK_x,			{ ZXK_x } },
		{ SDLK_c,			{ ZXK_c } },
		{ SDLK_v,			{ ZXK_v } },
		{ SDLK_a,			{ ZXK_a } },
		{ SDLK_s,			{ ZXK_s } },
		{ SDLK_d,			{ ZXK_d } },
		{ SDLK_f,			{ ZXK_f } },
		{ SDLK_g,			{ ZXK_g } },
		{ SDLK_q,			{ ZXK_q } },
		{ SDLK_w,			{ ZXK_w } },
		{ SDLK_e,			{ ZXK_e } },
		{ SDLK_r,			{ ZXK_r } },
		{ SDLK_t,			{ ZXK_t } },
		{ SDLK_1,			{ ZXK_1 } },
		{ SDLK_2,			{ ZXK_2 } },
		{ SDLK_3,			{ ZXK_3 } },
		{ SDLK_4,			{ ZXK_4 } },
		{ SDLK_5,			{ ZXK_5 } },
		{ SDLK_0,			{ ZXK_0 } },
		{ SDLK_9,			{ ZXK_9 } },
		{ SDLK_8,			{ ZXK_8 } },
		{ SDLK_7,			{ ZXK_7 } },
		{ SDLK_6,			{ ZXK_6 } },
		{ SDLK_p,			{ ZXK_p } },
		{ SDLK_o,			{ ZXK_o } },
		{ SDLK_i,			{ ZXK_i } },
		{ SDLK_u,			{ ZXK_u } },
		{ SDLK_y,			{ ZXK_y } },
		{ SDLK_RETURN,		{ ZXK_ENTER } },
		{ SDLK_l,			{ ZXK_l } },
		{ SDLK_k,			{ ZXK_k } },
		{ SDLK_j,			{ ZXK_j } },
		{ SDLK_h,			{ ZXK_h } },
		{ SDLK_SPACE,		{ ZXK_SPACE } },
		{ SDLK_LALT,		{ ZXK_SYMS } },
		{ SDLK_RALT,		{ ZXK_SYMS } },
		{ SDLK_LCTRL,		{ ZXK_SYMS } },
		{ SDLK_RCTRL,		{ ZXK_SYMS } },
		{ SDLK_m,			{ ZXK_m } },
		{ SDLK_n,			{ ZXK_n } },
		{ SDLK_b,			{ ZXK_b } },
	};

	// Map a ZX keycode to a row and column
	// These represent the physical keys on a 48K Spectrum
	//
	map<SDL_Keycode, Key> zxkeys = {
		{ ZXK_CAPS,		{ 0xFE, COL0 } },
        { ZXK_z,		{ 0xFE, COL1 } },
        { ZXK_x,		{ 0xFE, COL2 } },
        { ZXK_c,		{ 0xFE, COL3 } },
        { ZXK_v,		{ 0xFE, COL4 } },
        { ZXK_a,		{ 0xFD, COL0 } },
        { ZXK_s,		{ 0xFD, COL1 } },
        { ZXK_d,		{ 0xFD, COL2 } },
        { ZXK_f,		{ 0xFD, COL3 } },
        { ZXK_g,		{ 0xFD, COL4 } },
        { ZXK_q,		{ 0xFB, COL0 } },
        { ZXK_w,		{ 0xFB, COL1 } },
        { ZXK_e,		{ 0xFB, COL2 } },
        { ZXK_r,		{ 0xFB, COL3 } },
        { ZXK_t,		{ 0xFB, COL4 } },
        { ZXK_1,		{ 0xF7, COL0 } },
        { ZXK_2,		{ 0xF7, COL1 } },
        { ZXK_3,		{ 0xF7, COL2 } },
        { ZXK_4,		{ 0xF7, COL3 } },
        { ZXK_5,		{ 0xF7, COL4 } },
        { ZXK_0,		{ 0xEF, COL0 } },
        { ZXK_9,		{ 0xEF, COL1 } },
        { ZXK_8,		{ 0xEF, COL2 } },
        { ZXK_7,		{ 0xEF, COL3 } },
        { ZXK_6,		{ 0xEF, COL4 } },
        { ZXK_p,		{ 0xDF, COL0 } },
        { ZXK_o,		{ 0xDF, COL1 } },
        { ZXK_i,		{ 0xDF, COL2 } },
        { ZXK_u,		{ 0xDF, COL3 } },
        { ZXK_y,		{ 0xDF, COL4 } },
		{ ZXK_ENTER,	{ 0xBF, COL0 } },
        { ZXK_l,		{ 0xBF, COL1 } },
        { ZXK_k,		{ 0xBF, COL2 } },
        { ZXK_j,		{ 0xBF, COL3 } },
        { ZXK_h,		{ 0xBF, COL4 } },
        { ZXK_SPACE,	{ 0x7F, COL0 } },
		{ ZXK_SYMS,		{ 0x7F, COL1 } },
        { ZXK_m,		{ 0x7F, COL2 } },
        { ZXK_n,		{ 0x7F, COL3 } },
        { ZXK_b,		{ 0x7F, COL4 } },
	};
};