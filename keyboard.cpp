//
// Title:	        Spectrum 48K keyboard
// Description:		Spectrum 48K keyboard scanning routines
// Author:	        Dean Belfield
// Created:	        25/05/2026
// Last Updated:	28/05/2026
//
// Modinfo:

#include "keyboard.h"

Keyboard::Keyboard(Ports* ports) : ports(ports)
{
}

Keyboard::~Keyboard() {
}

void Keyboard::press(SDL_Keycode sym, bool pressed) {
    switch(sym) {
        case SDLK_LSHIFT: port(0xFE, 0, pressed); break;
        case SDLK_z: port(0xFE, 1, pressed); break;
        case SDLK_x: port(0xFE, 2, pressed); break;
        case SDLK_c: port(0xFE, 3, pressed); break;
        case SDLK_v: port(0xFE, 4, pressed); break;

        case SDLK_a: port(0xFD, 0, pressed); break;
        case SDLK_s: port(0xFD, 1, pressed); break;
        case SDLK_d: port(0xFD, 2, pressed); break;
        case SDLK_f: port(0xFD, 3, pressed); break;
        case SDLK_g: port(0xFD, 4, pressed); break;

        case SDLK_q: port(0xFB, 0, pressed); break;
        case SDLK_w: port(0xFB, 1, pressed); break;
        case SDLK_e: port(0xFB, 2, pressed); break;
        case SDLK_r: port(0xFB, 3, pressed); break;
        case SDLK_t: port(0xFB, 4, pressed); break;

        case SDLK_1: port(0xF7, 0, pressed); break;
        case SDLK_2: port(0xF7, 1, pressed); break;
        case SDLK_3: port(0xF7, 2, pressed); break;
        case SDLK_4: port(0xF7, 3, pressed); break;
        case SDLK_5: port(0xF7, 4, pressed); break;

        case SDLK_0: port(0xEF, 0, pressed); break;
        case SDLK_9: port(0xEF, 1, pressed); break;
        case SDLK_8: port(0xEF, 2, pressed); break;
        case SDLK_7: port(0xEF, 3, pressed); break;
        case SDLK_6: port(0xEF, 4, pressed); break;

        case SDLK_p: port(0xDF, 0, pressed); break;
        case SDLK_o: port(0xDF, 1, pressed); break;
        case SDLK_i: port(0xDF, 2, pressed); break;
        case SDLK_u: port(0xDF, 3, pressed); break;
        case SDLK_y: port(0xDF, 4, pressed); break;

        case SDLK_RETURN: port(0xBF, 0, pressed); break;
        case SDLK_l: port(0xBF, 1, pressed); break;
        case SDLK_k: port(0xBF, 2, pressed); break;
        case SDLK_j: port(0xBF, 3, pressed); break;
        case SDLK_h: port(0xBF, 4, pressed); break;

        case SDLK_SPACE: port(0x7F, 0, pressed); break;
        case SDLK_RSHIFT: port(0x7F, 1, pressed); break;
        case SDLK_m: port(0x7F, 2, pressed); break;
        case SDLK_n: port(0x7F, 3, pressed); break;
        case SDLK_b: port(0x7F, 4, pressed); break;
    }
}

void Keyboard::port(uint8_t p, uint8_t bit, bool pressed) {
	uint16_t b = 1 << bit;
	uint8_t* i = ports->getPortsIn();
	uint8_t  d = i[p];
    uint8_t  v = pressed ? d &= ~b : d |= b;

	i[p] = v;   // Write the value to the correct port
    i[0] = v;   // And a copy for port 0x00FE for routines that check for any key being pressed
}
