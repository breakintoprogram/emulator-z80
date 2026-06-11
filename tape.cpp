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
	uint16_t  blockSize;
	uint8_t   mark;	
    do {
    	file.read((char *)&blockSize, 2);	
		mark = file.peek();
        tape.push_back(make_unique<ToneSegment>(ulaPort, 2168, mark == 0 ? 8063 : 3223));
        tape.push_back(make_unique<PulseSegment>(ulaPort, 667, 735));
        tape.push_back(make_unique<DataSegment>(ulaPort, file, blockSize, 855, 1710));
        tape.push_back(make_unique<DelaySegment>(ulaPort, 1000));
    	bytesRemaining -= (blockSize + 2);			
    }
    while(bytesRemaining > 0);
    file.close();
    return true;
}

void Tape::play(uint16_t tStates) {
    if(tape.size() > 0) {
        if(tape[0]->isFinished()) {
            tape.erase(tape.begin());
        }
        else {
            tape[0]->play(tStates);
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
ToneSegment::ToneSegment(uint8_t* ulaPort, int16_t pulseWidth, int16_t pulseLength) :
    TapeSegment(ulaPort),
    pulseWidth(pulseWidth),
    pulseLength(pulseLength),
    count(pulseWidth),
    bit(false)
{
}

void ToneSegment::play(uint16_t tStates) {
    count -= tStates;
    if(count <= 0) {
        count += pulseWidth;
        if(bit) {
			finished = (pulseLength-- == 0);
        }
        bit = !bit;
    }
    writeBit(bit);
}

// Inherited pulse segment class
//
PulseSegment::PulseSegment(uint8_t* ulaPort, int16_t pulseWidth0, int16_t pulseWidth1) :
    TapeSegment(ulaPort),
    pulseWidth0(pulseWidth0),
    pulseWidth1(pulseWidth1)
{
}

void PulseSegment::play(uint16_t tStates) {
    if(pulseWidth0 > 0) {
        writeBit(0);
        pulseWidth0 -= tStates;
        return;
    }
    if(pulseWidth1 > 0) {
        writeBit(1);
        pulseWidth1 -= tStates;
        return;
    }
    finished = true;
}

// Inherited delay segment class
//
DelaySegment::DelaySegment(uint8_t* ulaPort, int16_t delay) :
    TapeSegment(ulaPort),
    delay(delay)
{
}

void DelaySegment::play(uint16_t tStates) { 
    delay -= tStates;
    finished = (delay <= 0);
}

// Inherited data segment class
//
DataSegment::DataSegment(uint8_t* ulaPort, ifstream& file, uint16_t blockSize, int16_t pulseWidth0, int16_t pulseWidth1) :
    TapeSegment(ulaPort),
    file(file),
    pulseWidth0(pulseWidth0),
    pulseWidth1(pulseWidth1),
    bitCount(0),
    pulseCount(0),
    bit(true)
{
    for(int i = 0; i < blockSize; i++) {
        data.push_back(file.get());
    }
}

void DataSegment::play(uint16_t tStates) {    
    pulseCount -= tStates;
    if(pulseCount <= 0) {                   // If the pulse has ended (or not started) then
        if(bit) {                           // If we've finished (or not started) playing the square wave
            if(bitCount > 0) {              // If there are more bits to process then
                bits <<= 1;                 // Shift onto the next bit
                bitCount--;                 // Decrement the bit count
            }
            else {                          // Otherwise get the next byte to process
                if(data.size() == 0) {      // Are there any more bytes?
                    finished = true;        // No, so we're done here
                    return;
                }
                bits = data[0];             // Get the next 8 bits
                bitCount = 8;               // Set the bit count
                data.erase(data.begin());   // Advance the data onto the next byte ready for next time
            }
        }
        //
        // This bit calculates the pulse width; 0s and 1s are
        // represented by different widths of square waves
        //
        pulseCount += ((bits & 0x80) ? pulseWidth1 : pulseWidth0);  
        bit = !bit;                         // Flip the bit
    }
    writeBit(bit);
}

