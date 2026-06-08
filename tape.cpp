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
    tape()
{
}

bool Tape::open(string filename) {
	if (!filesystem::exists(filename)) {
		return false;
	}
	uintmax_t filesize = filesystem::file_size(filename);
    ifstream file(filename, ios::binary);
    return openTAP(file, filesize);
}

bool Tape::openTAP(ifstream& file, uintmax_t filesize) {
    file.close();
    tape.push_back(make_unique<ToneSegment>(ulaPort, 2168/4.33, 8063/4.33));
    return true;
}

void Tape::play() {
    if(tape.size() > 0) {
        if(!tape[0]->isFinished()) {
            tape[0]->play();
        }
    }
}

TapeSegment::TapeSegment(uint8_t* ulaPort) : ulaPort(ulaPort)
{
}

bool TapeSegment::isFinished() {
    return finished;
}

void TapeSegment::writeBit(uint8_t bit) {
    if(bit < 2) {
        ulaPort[0x7f] &= 0b10111111;
        ulaPort[0x7f] |= bit << 6;
    }
}

ToneSegment::ToneSegment(uint8_t* ulaPort, uint16_t pulseWidth, uint16_t pulseLength) :
    TapeSegment(ulaPort),
    pulseWidth(pulseWidth),
    pulseLength(pulseLength),
    count(pulseWidth),
    bit(false)
{
}

void ToneSegment::play() {
    writeBit(bit);
    count--;
    if(count == 0) {
        count = pulseWidth;
        bit = !bit;
        if(!bit) {
            pulseLength--;
            if(pulseLength == 0) {
                finished = true;
            }
        }
    }
}


