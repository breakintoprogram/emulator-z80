//
// Title:	        Spectrum 48K Ports
// Description:		Provides read/write functionality for Z80 ports
// Author:	        Dean Belfield
// Created:	        02/06/2026
// Last Updated:	02/06/2026
//
// Modinfo:

#include "ports.h"

Ports::Ports() :
	floating(0xFF)
{
	for(int i=0; i<=255; i++) {
        ports_in[i] = 0b10111111;
    }
	ports_out[0] = 0xFF;
}

uint8_t Ports::in(uint16_t address) {
	if ((address & 0x0001) == 0) {		// Only interested in even numbered ports
		return ports_in[address >> 8];	// Return the port value
	}
	return floating;					// Odd ports return the floating bus value
}

void Ports::out(uint16_t address, uint8_t data) {
	ports_out[0] = data;
}

void Ports::setFloating(uint8_t data) {
	floating = data;
}

uint8_t* Ports::getPortsIn() {
    return ports_in;
}

uint8_t* Ports::getPortsOut() {
	return ports_out;
}