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
// - Pulse: L:2168T H:2168T repeated for the lead-in tone; 8063 half-pulses for header lead-in or 3223 for the subsequent data block lead-in
// - Pulse: L: 667T H: 735T a single pulse indicating the start of the data section
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
        tape.push_back(make_unique<PulseSegment>(ulaPort, 2168, 2168, mark == 0 ? 8063 : 3223));	// The long or short lead-in tone
        tape.push_back(make_unique<PulseSegment>(ulaPort, 667, 735, 2));                       		// The start of data pulse
        tape.push_back(make_unique<DataSegment>(ulaPort, file, blockSize, 855, 1710));      		// The data itself
        tape.push_back(make_unique<TapeSegment>(ulaPort, 1000));                           		    // A pause

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
    if (tape.size() > 0) {					// While there are segments to play
        if (tape[0]->play(tStates)) {		// Play the segment, if it flags it has finished then
	        tape.erase(tape.begin());		// Erase that segment; rinse, lather and repeat
        }
    }
}

// Base tape segment class, just plays nothing
// Parameters
// - ulaPort: Pointer to the ULA port address space
//
TapeSegment::TapeSegment(uint8_t* ulaPort, int16_t count) :
    ulaPort(ulaPort),
	count(count)
{
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

// Play a single time slice of empty tape
// Parameters:
// - tStates: Number of T-states to deduct from the pulse counters
// Returns:
// - false if still playing, true if finished
//
bool TapeSegment::play(uint16_t tStates) { 
    writeBit(0);                            // Just keep writing 0's out to the EAR port
    count -= tStates;                       // Adjust the delay
    return count <= 0;						// Flag when we're finished
}

// Inherited pulse segment class
// Parameters
// - ulaPort: Pointer to the ULA port address space
// - pulseWidth0: Width of the low pulse (in T-states)
// - pulseWidth1: Width of the high pulse (in T-states)
//
PulseSegment::PulseSegment(uint8_t* ulaPort, int16_t pulseWidth0, int16_t pulseWidth1, int16_t pulseCount) :
    TapeSegment(ulaPort, pulseWidth0),
    pulseWidth0(pulseWidth0),
    pulseWidth1(pulseWidth1),
	pulseCount(pulseCount),
	bit(false)
{
}

// Play a single time slice of the pulse segment
// Parameters:
// - tStates: Number of T-states to deduct from the pulse counters
// Returns:
// - false if still playing, true if finished
//
bool PulseSegment::play(uint16_t tStates) {
	count -= tStates;
	if (count <= 0) {
		if (pulseCount-- == 0) {
			return true;
		}
		bit = !bit;
		count = bit ? pulseWidth0 : pulseWidth1;
	}
	writeBit(bit);
	return false;
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
    TapeSegment(ulaPort, 0),
    file(file),
    pulseWidth0(pulseWidth0),
    pulseWidth1(pulseWidth1),
    bitMask(0),
    bit(true)
{
    for(int i = 0; i < blockSize; i++) {    // Write out an array of bytes to be process from the file stream
        data.push_back(file.get());
    }
}

// Play a single time slice of the dat segment
// Parameters:
// - tStates: Number of T-states to deduct from the pulse counters
// Returns:
// - false if still playing, true if finished
//
bool DataSegment::play(uint16_t tStates) {    
    count -= tStates;
    if(count <= 0) {                   		// If the pulse has ended (or not started) then
        if(bit) {                           // If we've finished (or not started) playing the square wave
            bitMask >>= 1;                  // Shift onto the next bit
            if(bitMask == 0) {              // If there are no more bits to process then
                if(data.size() == 0) {      // Are there any more bytes?
                    return true;			// No, so we're done here
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
        count += ((bits & bitMask) ? pulseWidth1 : pulseWidth0);  
        bit = !bit;                         // Flip the bit
    }
    writeBit(bit);                          // FInally write it out
	return false;
}

