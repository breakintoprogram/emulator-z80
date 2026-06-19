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

void Keyboard::press(SDL_Keycode sym, bool pressed) {
	Key k = keys[sym];
	port(k.port, k.column, pressed);
}

void Keyboard::port(uint8_t p, uint8_t bit, bool pressed) {
	uint8_t* i = ports->getPortsIn();
	uint8_t  d = i[p];
    uint8_t  v = pressed ? d &= ~bit : d |= bit;

	i[p] = v;   // Write the value to the correct port
    i[0] = v;   // And a copy for port 0x00FE for routines that check for any key being pressed
}
