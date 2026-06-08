//
// Title:	        Spectrum 48K Tape
// Description:		Provides basic tape fuctionality
// Author:	        Dean Belfield
// Created:	        08/06/2026
// Last Updated:	08/06/2026
//
// Modinfo:

#include "tape.h"

Tape::Tape(Ports* ports) :
    ulaPort(ports->getPortsIn()),
    state(0)
{
}

void Tape::writeBit(uint8_t bit) {
    if(bit < 2) {
        (*ulaPort) &= 0b10111111;
        (*ulaPort) |= bit << 6;
    }
}

void Tape::play() {
    switch(state) {
        case 0: { // Idle
            
        } break;
    }
}

bool Tape::open(string filename) {
	if (!filesystem::exists(filename)) {
		return false;
	}
	filesize = filesystem::file_size(filename);
    fs = ifstream(filename, ios::binary);
    return true;
}

void Tape::close() {
    fs.close();
}

