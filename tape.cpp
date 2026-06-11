//
// Title:	        Spectrum 48K Tape
// Description:		Provides basic tape fuctionality
// Author:	        Dean Belfield
// Created:	        08/06/2026
// Last Updated:	08/06/2026
//
// Modinfo:
 
// In this explanation the pulses are written as
//
// - L:xxxxT: Low pulse for xxxx T states
// - H:xxxxT: High pulse for xxxx T states
//
// The Sinclair LD-BYTES routine expects the tones to be written to the EAR port as follows:
//
// - Tone:  L:8063T H:8063T for the header lead in tone, or L:3223T for the shorter lead-in before the data block
// - Pulse: L: 667T H: 735T indicating the start of the data section
// - Data:  L: 855T H: 855T represents a zero, L:1710 H:1710 represents a 1
// - Delay: L:1000T
//
// The openTAP routine writes out an array of objects that will output the pulses in the correct sequence
// The play routine plays a time slice of the loader, and is passed how many T-states need to be accounted for in the timing

#include "tape.h"

// Constructor for the main Tape class
// Parameters:
// - ports: Pointer to a Ports object
//
Tape::Tape(Ports* ports) :
    ulaPort(ports->getPortsIn()),   // Pointer to the 256-byte ports array
    tape()                          // Empty vector for TapeSegment objects (ToneSegment, PulseSegment, DataSegment and DelaySegment)
{
}

// Open an emulator file and process it
// Parameters:
// - filename: path and filename of the tap file
// Returns:
// - true if the file could be opened and parsed, otherwise false
//
bool Tape::open(string filename) {
	if (!filesystem::exists(filename)) {
		return false;
	}
	uintmax_t filesize = filesystem::file_size(filename);
    ifstream file(filename, ios::binary);
    return openTAP(file, filesize);
}

// Open and process a TAP file
// Parameters:
// - file: A filestream pointing to the start of the TAP file
// - filesize: Size of the file in bytes
// Returns:
// - true if the file could be opened and parsed, otherwise false
//
bool Tape::openTAP(ifstream& file, uintmax_t filesize) {
    uintmax_t bytesRemaining = filesize;
	uint16_t  blockSize;
	uint8_t   mark;	
    do {
    	file.read((char *)&blockSize, 2);	    // Read in the data block size from the TAP file
		mark = file.peek();                     // The next byte is the type of block (0 = full lead-in, 1 = shorter data block lead-in)
        //
        // Push onto the tape array the correspoding objects for a single data block
        //
        tape.push_back(make_unique<ToneSegment>(ulaPort, 2168, mark == 0 ? 8063 : 3223));   // The long or short lead-in tone
        tape.push_back(make_unique<PulseSegment>(ulaPort, 667, 735));                       // The start of data pulse
        tape.push_back(make_unique<DataSegment>(ulaPort, file, blockSize, 855, 1710));      // The data itself
        tape.push_back(make_unique<DelaySegment>(ulaPort, 1000));                           // A pause

    	bytesRemaining -= (blockSize + 2);      // Decrease number of bytes remaining
    }
    while(bytesRemaining > 0);                  // Loop, then
    file.close();                               // Close the file
    return true;
}

// Play a single time slice of the loader
//
// This function steps through the tape object created in the open method and will execute the play method on the one
// at position 0 in the array until that element's isFinished method returns true. It will then remove that element
// from the array and rinse, lather and repeat until all the elements are done. Each object in the tape array is
// responsible for playing the appropriate tones.
//
// Parameters:
// - tStates: Number of T-states to deduct from the pulse counters
//
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
// Parameters
// - ulaPort: Pointer to the ULA port address space
//
TapeSegment::TapeSegment(uint8_t* ulaPort) :
    ulaPort(ulaPort),
    finished(false)
{
}

// Has this segment finished?
// Returns:
// - true if the segment has finished, otherwise false
//
bool TapeSegment::isFinished() {
    return finished;
}

// Write a bit out to the EAR port
// Parameters:
// - bit: Either 0 or 1, all other values are ignored
//
void TapeSegment::writeBit(uint8_t bit) {
    if(bit < 2) {
        ulaPort[0x7f] &= 0b10111111;
        ulaPort[0x7f] |= bit << 6;
    }
}

// Inherited lead-in tone segment class
// Parameters
// - ulaPort: Pointer to the ULA port address space
// - pulseWidth: Width (in T-states) of the tone pulse
// - pulseLength: Number of pulses
//
ToneSegment::ToneSegment(uint8_t* ulaPort, int16_t pulseWidth, int16_t pulseLength) :
    TapeSegment(ulaPort),
    pulseWidth(pulseWidth),
    pulseLength(pulseLength),
    count(pulseWidth),
    bit(false)
{
}

// Play a single time slice of the tone segment
// Parameters:
// - tStates: Number of T-states to deduct from the pulse counters
//
void ToneSegment::play(uint16_t tStates) {
    count -= tStates;                           // Decrease the pulse count timer
    if(count <= 0) {                            // If less than zero then we need to flip the bit
        count += pulseWidth;                    // Reset the pulse counter timer for the next pulse
        if(bit) {                               // If we've done a full pulse then
			finished = (pulseLength-- == 0);    // Decrease the pulse length by 1 and flag finished when done
        }
        bit = !bit;                             // Flip the bit
    }
    writeBit(bit);                              // Write to the EAR port
}

// Inherited pulse segment class
// Parameters
// - ulaPort: Pointer to the ULA port address space
// - pulseWidth0: Width of the low pulse (in T-states)
// - pulseWidth1: Width of the high pulse (in T-states)
//
PulseSegment::PulseSegment(uint8_t* ulaPort, int16_t pulseWidth0, int16_t pulseWidth1) :
    TapeSegment(ulaPort),
    pulseWidth0(pulseWidth0),
    pulseWidth1(pulseWidth1)
{
}

// Play a single time slice of the pulse segment
// Parameters:
// - tStates: Number of T-states to deduct from the pulse counters
//
void PulseSegment::play(uint16_t tStates) {
    if(pulseWidth0 > 0) {                       // Do the low pulse if > 0
        writeBit(0);
        pulseWidth0 -= tStates;
        return;                           
    }
    if(pulseWidth1 > 0) {                       // Do the high pulse if > 0
        writeBit(1);
        pulseWidth1 -= tStates;
        return;
    }
    finished = true;                            // Flag finished
}

// Inherited delay segment class
// Parameters
// - ulaPort: Pointer to the ULA port address space
// - delay: Width of the delay (in T-states)
//
DelaySegment::DelaySegment(uint8_t* ulaPort, int16_t delay) :
    TapeSegment(ulaPort),
    delay(delay)
{
}

// Play a single time slice of the delay segment
// Parameters:
// - tStates: Number of T-states to deduct from the pulse counters
//
void DelaySegment::play(uint16_t tStates) { 
    writeBit(0);                                // Just keep writing 0's out to the EAR port
    delay -= tStates;                           // Adjust the delay
    finished = (delay <= 0);                    // Flag when we're finished
}

// Inherited pulse segment class
// Parameters
// - ulaPort: Pointer to the ULA port address space
// - file: Pointer to the start of the Spectrum LOAD data
// - blockSize: Size of the data blocker
// - pulseWidth0: Width of a 0 bit
// - pulseWidth1: Width of a 1 bit
//
DataSegment::DataSegment(uint8_t* ulaPort, ifstream& file, uint16_t blockSize, int16_t pulseWidth0, int16_t pulseWidth1) :
    TapeSegment(ulaPort),
    file(file),
    pulseWidth0(pulseWidth0),
    pulseWidth1(pulseWidth1),
    bitMask(0),
    pulseCount(0),
    bit(true)
{
    for(int i = 0; i < blockSize; i++) {    // Write out an array of bytes to be process from the file stream
        data.push_back(file.get());
    }
}

void DataSegment::play(uint16_t tStates) {    
    pulseCount -= tStates;
    if(pulseCount <= 0) {                   // If the pulse has ended (or not started) then
        if(bit) {                           // If we've finished (or not started) playing the square wave
            bitMask >>= 1;                  // Shift onto the next bit
            if(bitMask == 0) {              // If there are no more bits to process then
                if(data.size() == 0) {      // Are there any more bytes?
                    finished = true;        // No, so we're done here
                    return;
                }
                bits = data[0];             // Get the next 8 bits
                bitMask = 0x80;             // Set the bit mask
                data.erase(data.begin());   // Advance the data onto the next byte ready for next time
            }
        }
        //
        // This bit calculates the pulse width; 0s and 1s are
        // represented by different widths of square waves
        //
        pulseCount += ((bits & bitMask) ? pulseWidth1 : pulseWidth0);  
        bit = !bit;                         // Flip the bit
    }
    writeBit(bit);                          // FInally write it out
}

