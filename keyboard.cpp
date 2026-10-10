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

void Keyboard::keyDown(SDL_KeyboardEvent &key) {
	if (key.repeat) {							// Check if the key is repeating
		return;									// Ignore if it is; the emulated machine will handle that
	}
	uint8_t* i = ports->getPortsIn();			// Pointer to array of IN ports
	SDL_Keysym ks = key.keysym;					// The reported key press
	printf("KEYDOWN: %d,%d,%s\n", ks.sym, ks.mod, SDL_GetKeyName(ks.sym));	
	//
	// Get the correct lookup table
	//
	auto& keycodes = (ks.mod & KMOD_SHIFT) ? keycodes_shifted: keycodes_unshifted;
	//
	// Look up the SDL keycode and get a list of ZX keycodes
	//	
	auto it = keycodes.find(ks.sym);			// Does the key exist in the table?
	if (it == keycodes.end()) {					// No, so don't do anything
		return;
	}	
	pressed[ks.sym] = ks;						// Store the key as being pressed
	//
	// Finally iterate the ZX keycodes and update the keyboard ports
	//
	for (ZX_Keycode k : it->second) {
		Key key = zxkeys[k];					// Look up the ZX keycode row and column
		i[key.port] &= ~key.bit;				// Write the value to the correct port
		i[0] &= ~key.bit;						// And a copy for port 0x00FE for routines that check for any key being pressed
	}	
}

void Keyboard::keyUp(SDL_KeyboardEvent &key) {
	SDL_Keycode sym = key.keysym.sym;			// The virtual key code released
	//
	// Lookup which key press this relates to
	//
	if (pressed.count(sym) == 0) {				// The key is not in the pressed table so
		return;									// do nothing
	}
    SDL_Keysym ks = pressed[sym];			 	// Get the key
    pressed.erase(sym);							// Erase it from the pressed table
	printf("KEYUP: %d,%d,%s ... %zu\n", ks.sym, ks.mod,  SDL_GetKeyName(ks.sym), pressed.size());

	uint8_t* i = ports->getPortsIn();			// Pointer to array of IN ports
	//
	// Get the correct lookup table
	//
	auto& keycodes = (ks.mod & KMOD_SHIFT) ? keycodes_shifted: keycodes_unshifted;
	//
	// Look up the SDL keycode and get a list of ZX keycodes
	//	
	auto it = keycodes.find(ks.sym);			// Does the key exist in the table?
	if (it == keycodes.end()) {					// No, so don't do anything
		return;
	}	
	//
	// Finally iterate the ZX keycodes and update the keyboard ports
	//
	for (ZX_Keycode k : it->second) {
		Key key = zxkeys[k];					// Look up the ZX keycode row and column
		i[key.port] |= key.bit;					// Write the value to the correct port
		i[0] |= key.bit;						// And a copy for port 0x00FE for routines that check for any key being pressed
	}	
}