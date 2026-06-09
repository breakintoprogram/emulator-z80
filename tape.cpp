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
    uintmax_t bytesRemaining = filesize;
    do {
        tape.push_back(make_unique<ToneSegment>(ulaPort, TSPEED(2168), TSPEED(8063)));
        tape.push_back(make_unique<DataSegment>(ulaPort, file, bytesRemaining));
    }
    while(bytesRemaining > 0);
    file.close();
    return true;
}

void Tape::play() {
    if(tape.size() > 0) {
        if(tape[0]->isFinished()) {
            tape.erase(tape.begin());
        }
        else {
            tape[0]->play();
        }
    }
}

// Base tape segment class
//
TapeSegment::TapeSegment(uint8_t* ulaPort) :
    ulaPort(ulaPort),
    finished(false)
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

// Inherited lead-in tone segment class
//
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

// Inherited data segment class
//
DataSegment::DataSegment(uint8_t* ulaPort, ifstream& file, uintmax_t& bytesRemaining) :
    TapeSegment(ulaPort),
    file(file)
{
    uint16_t length;
    file.read((char *)&length, 2);
    for(int i = 0; i < length; i++) {
        data.push_back(file.get());
    }
    bytesRemaining -= (length + 2);
}

void DataSegment::play() {
    finished = true;
}

