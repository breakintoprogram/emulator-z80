//
// Title:	        Spectrum 48K keyboard
// Description:		Spectrum 48K keyboard scanning routines
// Author:	        Dean Belfield
// Created:	        25/05/2026
// Last Updated:	04/09/2026
//
// Modinfo:
// 04/09/2025:		Added ZX_Keycodes

#include "keyboard.h"

Keyboard::Keyboard(Ports* ports) : ports(ports)
{
}

void Keyboard::press(SDL_Keycode sym, bool pressed) {
	uint8_t* i = ports->getPortsIn();		// Pointer to array of IN ports
	//
	// Look up the SDL keycode and get a list of ZX keycodes
	//
	auto it = keycodes.find(sym);			// Does the key exist in the table?
	if (it == keycodes.end()) {				// No, so don't do anything
		return;
	}
	//
	// Finally iterate the ZX keycodes and update the keyboard ports
	//
	for (ZX_Keycode k : it->second) {
		Key key = zxkeys[k];				// Look up the ZX keycode row and column
		uint8_t d = i[key.port];			// The current data for that keys port
		//
		// If the key is pressed, then we need to AND (zero) out the bit
		// Otherwise if the key is not pressed with OR (one) in the bit
		//
		uint8_t  v = pressed ? d &= ~key.bit : d |= key.bit;

		i[key.port] = v;					// Write the value to the correct port
		i[0] = v;							// And a copy for port 0x00FE for routines that check for any key being pressed
	}
}