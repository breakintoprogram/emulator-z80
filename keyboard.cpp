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
	uint8_t* i = ports->getPortsIn();	// Pointer to array of IN ports
	Key*     k = &keys[sym];			// Pointer to the currently pressed PC key
	uint8_t  d = i[k->port];			// The current data for that keys port
	//
	// If the key is pressed, then we need to AND (zero) out the bit
	// Otherwise if the key is not pressed with OR (one) in the bit
	//
    uint8_t  v = pressed ? d &= ~k->bit : d |= k->bit;

	i[k->port] = v;	// Write the value to the correct port
    i[0] = v;		// And a copy for port 0x00FE for routines that check for any key being pressed
}