//
// Title:	        Z80 CPU
// Description:		Z80 CPU emulation
// Author:	        Dean Belfield
// Created:	        22/05/2026
// Last Updated:	28/05/2026
//
// Modinfo:

#include "z80.h"

Z80::Z80(Mem* mem, Ports* ports, Logger* logger) :
	mem(mem),
	ports(ports),
	logger(logger),
	p(0),
	q(0),
	x(0),
	y(0),
	z(0),
	t(0),
	shift_IXY(0),
	data(0),
	breakpoints(),
	interrupt(0),
	halted(false),
	blockop(false),
	singleStep(false),
	trace(false)
{
}

// Add a breakpoint
//
void Z80::addBreakpoint(uint16_t a) {
	breakpoints.push_back(a);
}

bool Z80::getSingleStep() {
	return singleStep;
}

void Z80::setSingleStep(bool value) {
	singleStep = value;
}

bool Z80::getTrace() {
	return trace;
}

void Z80::setTrace(bool value) {
	trace = value;
}

uint32_t Z80::getT() {
	return t;
}

void Z80::setT(uint16_t value) {
	t = value;
}

void Z80::incT(uint16_t value) {
	t += value;
}

// Do the NMI
//
void Z80::interruptRequest() {
	interrupt = true;
}

// Reset the CPU
//
void Z80::reset()
{
	reg.PC = 0;
}

// Debug dump
//
void Z80::dump() {
	logger->getStream() << setfill('0') << hex
	<< "PC: " << setw(4) << reg.PC << endl
	<< "SP: " << setw(4) << reg.SP << endl
	<< "BC: " << setw(4) << reg.BC.W << " "
	<< "DE: " << setw(4) << reg.DE.W << " "
	<< "HL: " << setw(4) << reg.HL.W << " "
	<< "A: "  << setw(2) << (uint16_t)reg.AF.A << " "
	<< "F: ["
	<< (reg.AF.S ? 'S' : '-')
	<< (reg.AF.Z ? 'Z' : '-')
	<< (reg.AF.Y ? 'Y' : '-')
	<< (reg.AF.B ? 'H' : '-')
	<< (reg.AF.X ? 'X' : '-')
	<< (reg.AF.P ? 'P' : '-')
	<< (reg.AF.N ? 'N' : '-')
	<< (reg.AF.C ? 'C' : '-')
	<< "]" << endl
	<< "BC: " << setw(4) << reg.BC_.W << " "
	<< "DE: " << setw(4) << reg.DE_.W << " "
	<< "HL: " << setw(4) << reg.HL_.W << " "
	<< "A: "  << setw(2) << (uint16_t)reg.AF_.A << " "
	<< "F: ["
	<< (reg.AF_.S ? 'S' : '-')
	<< (reg.AF_.Z ? 'Z' : '-')
	<< (reg.AF_.Y ? 'Y' : '-')
	<< (reg.AF_.B ? 'H' : '-')
	<< (reg.AF_.X ? 'X' : '-')
	<< (reg.AF_.P ? 'P' : '-')
	<< (reg.AF_.N ? 'N' : '-')
	<< (reg.AF_.C ? 'C' : '-')
	<< "] (alt)" << endl
	<< "IX: " << setw(4) << reg.IX.W << endl
	<< "IY: " << setw(4) << reg.IY.W << endl;
}

// Run one CPU cycle
//
void Z80::run() {
	if (halted) {			// If we are halted
		interrupts();		// Handle any interrupts
		setT(4);			// Effectively running NOPs
	}
	else {					// CPU is not halted at this point
		debug();
		interrupts();		// Handle any interrupts
		do {				// Go into a loop running a single cycle
			fetch();		// Fetch the next value from the PC
			//
			// The code could have multiple DD or FD values before an opcode, we're only
			// interested in the last one before the opcode
			//
			while (data == 0xDD || data == 0xFD) {
				shift_IXY = ((data & 0b00100000) >> 5) + 1;
				fetch();
			}
			decode();		// Decode the opcode
			execute();		// Execute it
		} while (blockop);	// Loop if doing a block operation (LDIR, etc)
		//
		// Tail the debug output with the T-state value
		//
		if (trace) {
			logger->setT(getT());
			logger->output();
		}
		//
		// Throw an error if the T-state value is 0, indicates an emulator issue
		//
		if (getT() == 0) {
			throw runtime_error("No T-states registered for this instruction");
		}
	}
}

// Output some debugging preamble
//
void Z80::debug() {
	if (trace) {
		logger->clear();
		logger->setPC(reg.PC);
	}
	if (find(breakpoints.begin(), breakpoints.end(), reg.PC) != breakpoints.end()) {
		setSingleStep(true);
	}
}

// Fetch an opcode
//
void Z80::fetch()
{
	data = mem->readByte(reg.PC++);
	if (!blockop && trace) {
		logger->addData(data);
	}
}

// Private helper method to fetch a word
//
uint16_t Z80::fetchWord() {
	fetch();
	uint16_t w = data;		// The LSB
	fetch();
	return w | (data << 8);	// The MSB combined with the LSB
}

// Decode the instruction
//
void Z80::decode()
{
	x = (data & 0xC0) >> 6;	// 0b11000000
	y = (data & 0x38) >> 3;	// 0b00111000
	z = (data & 0X07);		// 0b00000111
	p = y >> 1;				// 0b00110000
	q = y & 1; 				// 0b00001000
}

// Execute the instruction
//
void Z80::execute()
{
	if (data == 0xCB) {
		execute_CB();
		reg.incR(2);
	}
	else if (data == 0xED) {
		execute_ED();
		reg.incR(2);
	}
	else {
		xz_t f = lut_xz[x][z];
		(this->*f)();
		reg.incR(1);
	}
	shift_IXY = 0;
}

// Handle interrupts
// NB: The maskable interrupt has not been implemented
//
void Z80::interrupts() {
	//
	// Handle the interrupts
	//
	if (interrupt) {						// If an interrupt has been triggered
		interrupt = false;					// Acknowledge
		if (reg.IFF1) {						// Are interrupts enabled?
			reg.IFF1 = false;				// Yes, so disable interrupts
			halted = false;					// Set CPU to be running
			if (reg.IM == 0) {
				throw runtime_error("interrupts: IM 0 not implemented");
			}
			else if (reg.IM == 1) {
				push(reg.PC);				// Push the current program counter on the stack
				reg.PC = 0x38;				// Set the program counter to the maskable interrupt routine
			}
			else if (reg.IM == 2) {
				push(reg.PC);
				reg.PC = mem->readWord(reg.I << 8);
			}
			else {
				throw runtime_error("interrupts: invalid IM");
			}
		}
	}
}

void Z80::execute_CB() {
	//
	// Special case for DD/FD prefixes
	// The index has been stored in index_CB
	// This will have immediately followed the DDCB or FDCB shift pair
	//
	if (shift_IXY != 0) {		
		fetch();								// If it is a DDCB or FDCB opcode shift then
		uint8_t i = data;						// The next byte is the index							
		uint16_t a = getIXY(shift_IXY, i);		// Calculate the address offset by index
		fetch();								// Fetch the instruction
		decode();								// And decode
		uint8_t* r = lut_r[0][z];	
		uint8_t  s = 1<<y;	
		uint8_t  b;
		switch(x) {
			case 0: { // ROT
				rot_t f = lut_rot[y];
				b = mem->readByte(a);
				(reg.*f)(&b);
				mem->write(a, b);
				if (r) {
					*r = b;
					LOG_OPCODE("LD " << txt_r[0][z] << ",");
				}
				reg.AF.B = 0;
				reg.AF.N = 0;
				setT(23);
				LOG_OPCODE(txt_rot[y] << " (" << txt_rp1[shift_IXY][2] << "+" << (uint16_t)i << ")");
			} break;
			case 1: { // BIT
				b = mem->readByte(a) & s;
				reg.AF.Z = (b == 0);
				reg.AF.S = (y == 7 && b != 0);
				reg.AF.P = reg.AF.Z;
				reg.AF.B = 1;
				reg.AF.N = 0;		
				setT(20);			
				LOG_OPCODE("BIT " << (uint16_t)y << ",(" << txt_rp1[shift_IXY][2] << "+" << (uint16_t)i << ")");
			} break;
			case 2: { // RES
				b = mem->readByte(a) & ~s;
				mem->write(a, b);
				if (r) {
					*r = b;
					LOG_OPCODE("LD " << txt_r[0][z] << ",");
				}
				setT(23);
				LOG_OPCODE("RES " << (uint16_t)y << ",(" << txt_rp1[shift_IXY][2] << "+" << (uint16_t)i << ")");
			} break;
			case 3: { // SET
				b = mem->readByte(a) | s;
				mem->write(a, b);
				if (r) {
					*r = b;
					LOG_OPCODE("LD " << txt_r[0][z] << ",");
				}	
				setT(23);	 
				LOG_OPCODE("SET " << (uint16_t)y << ",(" << txt_rp1[shift_IXY][2] << "+" << (uint16_t)i << ")");
			} break;
		}
	}
	//
	// Normal CB operations
	//
	else {
		fetch();								// Fetch the instruction
		decode();								// And decode
	 	uint8_t*  r = lut_r[0][z];				// Look up the register; NULL if (HL)
		uint8_t   s = 1<<y;
		uint8_t   b;
		switch(x) {
			case 0: { // ROT
				rot_t f = lut_rot[y];			// Look up the ROT operation
				if (r) {						// If it is a register then
					(reg.*f)(r);				// Execute the function on the register
					setT(8);
				}
				else {
					uint16_t a = getInd(0);		// Get the memory address
					b = mem->readByte(a);		// Get the byte
					(reg.*f)(&b);				// Execute the function on the memory location				
					mem->write(a, b);
					setT(15);
				}
				LOG_OPCODE(txt_rot[y] << " " << txt_r[0][z]);
			} break;
			case 1: { // BIT y,r[z]				// This is a read only operation so no need for ROM check
				b = r ? *r : mem->readByte(getInd(0));
				b &= s;							// Mask it out
				reg.AF.Z = (b == 0);
				reg.AF.S = (y == 7 && b != 0);
				reg.AF.P = reg.AF.Z;
				reg.AF.B = 1;
				reg.AF.N = 0;	
				reg.AF.X = (y == 3 && b != 0);	
				reg.AF.Y = (y == 5 && b != 0);	
				setT(r ? 8 : 12);		
				LOG_OPCODE("BIT " << (uint16_t)y << "," << txt_r[0][z]);
			} break;
			case 2: { // RES y,r[z]
				if (r) {
					*r &= ~s;					// r is a register pointer
					setT(8);
				}
				else {
					uint16_t a = getInd(0);		// do the operation in memory
					b = mem->readByte(a) & ~s;
					mem->write(a, b);		
					setT(15);		
				}
				LOG_OPCODE("RES " << (uint16_t)y << "," << txt_r[0][z]);
			} break;
			case 3: { // SET y,r[z]
				if (r) {
					*r |= s;					// r is a register pointer
					setT(8);
				}
				else {
					uint16_t a = getInd(0);		// do the operation in memory
					b = mem->readByte(a) | s;
					mem->write(a, b);		
					setT(15);			
				}		
				LOG_OPCODE("SET " << (uint16_t)y << "," << txt_r[0][z]);
			} break;
		}
	}
}

void Z80::execute_ED() {
	fetch();
	decode();

	switch(x) {
		case 1: {
			switch(z) {
				case 0: { // IN (C)
					uint8_t* r = lut_r[0][y];
					uint8_t b = ports->in(reg.BC.W);
					if (r) {
						*r = b;
					}
					reg.setFlagsSZP(b);
					reg.setFlagsXY(b);
					reg.AF.B = 0;
					reg.AF.N = 0;
					setT(12);
					LOG_OPCODE("IN (C)");
				} break;	
				case 1: { // OUT (C)
					uint8_t* r = lut_r[0][y];
					ports->out(reg.BC.W, r ? *r : 0);
					setT(12);
					LOG_OPCODE("OUT (C)");
				} break;
				case 2: { // ADC/SBC
					uint16_t* rp1 = lut_rp1[0][2]; // HL
					uint16_t* rp2 = lut_rp1[0][p]; // The other register pair
					if (q == 0) {
						reg.sbc(rp1, *rp2);
						LOG_OPCODE("SBC HL," << txt_rp1[0][p]);
					}
					else {
						reg.adc(rp1, *rp2);
						LOG_OPCODE("ADC HL," << txt_rp1[0][p]);
					}
					setT(15);
				} break;
				case 3: { // Load register pair from/to immediate address
					uint16_t* rp = lut_rp1[0][p];
					uint16_t  dd = fetchWord();
					if (q ==0) {
						mem->write(dd, *rp);	
						LOG_OPCODE("LD (" << setw(4) << dd << ")," << txt_rp1[0][p]);
					}
					else {
						*rp = mem->readWord(dd);	
						LOG_OPCODE("LD " << txt_rp1[0][p] << ",(" << setw(4) << dd << ")");
					}
					setT(p == 2 ? 16 : 20);
				} break;
				case 4: { // NEG
					reg.neg();
					setT(4);
					LOG_OPCODE("NEG");
				} break;
				case 5: { // RETI/RETN
					reg.PC = pop();
					if (y != 1) { // Check for RETN
						reg.IFF1 = reg.IFF2;
						LOG_OPCODE("RETN");
					}
					else {
						LOG_OPCODE("RETI");
					}
					setT(14);
				} break;
				case 6: { // IM
					switch(y & 3) {
						case 0: reg.IM = 0; break;
						case 1: reg.IM = 0; break; // IM 0/1
						case 2: reg.IM = 1; break;
						case 3: reg.IM = 2; break;
					}
					setT(8);
					LOG_OPCODE("IM " << reg.IM);
				} break;
				case 7: { // Assorted ops
					switch(y) {
						case 0: { // LD I,A
							reg.I = reg.AF.A;
							setT(9);
							LOG_OPCODE("LD I,A");
						} break;
						case 1: { // LD R,A
							reg.R = reg.AF.A;
							setT(9);
							LOG_OPCODE("LD R,A");
						} break;
						case 2: { // LD A,I
							reg.AF.A = reg.I;
							reg.setFlagsSZ(reg.AF.A);
							reg.setFlagsXY();
							reg.AF.P = reg.IFF2;
							reg.AF.B = 0;
							reg.AF.N = 0;
							setT(9);
							LOG_OPCODE("LD A,I");
						} break;
						case 3: { // LD A,R
							reg.AF.A = reg.R;
							reg.setFlagsSZ(reg.AF.A);
							reg.setFlagsXY();
							reg.AF.P = reg.IFF2;
							reg.AF.B = 0;
							reg.AF.N = 0;
							setT(9);
							LOG_OPCODE("LD A,R");
						} break;
						case 4: { // RRD
							uint8_t d = mem->readByte(reg.HL.W);
							uint8_t a = reg.AF.A;
							reg.AF.A = (reg.AF.A & 0xF0) | (d & 0x0F);
							d = ((a & 0x0F) << 4) | ((d & 0xF0) >> 4);
							mem->write(reg.HL.W, d);
							reg.setFlagsSZP(reg.AF.A);
							reg.setFlagsXY();
							reg.AF.B = 0;
							reg.AF.N = 0;
							setT(18);
							LOG_OPCODE("RRD");
						} break;
						case 5: { // RLD
							uint8_t d = mem->readByte(reg.HL.W);
							uint8_t a = reg.AF.A;
							reg.AF.A = (reg.AF.A & 0xF0) | ((d & 0xF0) >> 4);
							d = ((d & 0x0F) << 4) | (a & 0x0F);
							mem->write(reg.HL.W, d);
							reg.setFlagsSZP(reg.AF.A);
							reg.setFlagsXY();
							reg.AF.B = 0;
							reg.AF.N = 0;
							setT(18);
							LOG_OPCODE("RLD");
						} break;
					}
				} break;
			}
		} break;

		case 2: { // Block instructions
			if (z < 4 && y >= 4) {
				uint8_t i = y - 4;
				if (i < 2) {			// Is it a one-shot block instruction like LDI, IND?
					setT(16);			// Yes, so just set the T-states
				}
				else {					// Otherwise it's a block operation like LDIR, INDR
					if (blockop) {		// If we're already processing it then
						incT(21);		// Just increment the T-states
					}
					else {
						setT(16);		// Otherwise initialise the T-states and
						blockop = true;	// flag that we're in a block operation
						LOG_OPCODE(txt_bli[i][z]);
					}
				}
				bli_t f = lut_bli[i][z];	// Lookup the function pointer for the block instruction
				(this->*f)();			// And run it
			}	
			else {
				blockop = false;
				throw runtime_error("execute_ED: invalid block instruction");
			}
		} break;

		default: {
			blockop = false;			
			throw runtime_error("execute_ED: invalid opcode");
		} break;
	}
}

//
// X=0, Z=0: Relative jumps and assorted ops
//
void Z80::execute_x0z0()
{
	switch (y) {
		//
		// NOP
		//
		case 0:	{
			setT(4);
			LOG_OPCODE("NOP");
		} break;
		//
		// EX AF,AF'
		//
		case 1: {
			reg.exaf();
			setT(4);
			LOG_OPCODE("EX AF,AF'");
		} break;
		//
		// DJNZ n
		//
		case 2: {
			fetch();
			reg.BC.H--;
			if (reg.BC.H != 0) {
				reg.PC += (int8_t)data;
			}
			setT(13);
			LOG_OPCODE("DJNZ " << setw(4) << reg.PC);
		} break;
		//
		// JR n
		//
		case 3: {
			fetch();
			reg.PC += (int8_t)data;
			setT(12);
			LOG_OPCODE("JR " << setw(4) << reg.PC);
		} break;
		//
		// JR c,n
		//
		default: {
			fetch();							// Fetch the relative jump value
			cc_t     f = lut_cc[y-4];			// Look up the cc function
			bool     c = (reg.*f)();			// Get the condition
			uint16_t a = reg.PC + (int8_t)data; // Get the address
			if(c) {								// If the condition true then
				reg.PC = a;						// JR to the location
				setT(12);
			}
			else {
				setT(7);
			}
			LOG_OPCODE("JR " << txt_cc[y-4] << "," << setw(4) << a);
		} break;
	}
}

//
// X=0, Z=1: 16-bit load immediate/add
//
void Z80::execute_x0z1() {
	if (q == 0) {	// LD rr,n
		uint16_t* rp = lut_rp1[shift_IXY][p];
		uint16_t  dd = fetchWord();				
		*rp = dd;
		setT(shift_IXY ? 14 : 10);
		LOG_OPCODE("LD " << txt_rp1[shift_IXY][p] << "," << setw(4) << dd);
	}
	else {			// ADD HL,rr
		uint16_t* rp1 = lut_rp1[shift_IXY][2]; // HL/IX/IY
		uint16_t* rp2 = lut_rp1[shift_IXY][p]; // The other register pair
		reg.add(rp1, *rp2);
		setT(shift_IXY ? 15 : 11);
		LOG_OPCODE("ADD " << txt_rp1[shift_IXY][2] << "," << txt_rp1[shift_IXY][p]);
	}
}

//
// X=0, Z=2: Indirect load
//
void Z80::execute_x0z2() {
	if (q == 0) {
		switch(p) {
			case 0: { // LD (BC),A
				mem->write(reg.BC.W, reg.AF.A);
				setT(7);
				LOG_OPCODE("LD (BC),A");
			} break;
			case 1: { // LD (DE),A
				mem->write(reg.DE.W, reg.AF.A);
				setT(7);
				LOG_OPCODE("LD (DE),A");
			} break;
			case 2: { // LD (nn),HL/IX/IY 
				uint16_t* rp = lut_rp1[shift_IXY][2];
				uint16_t  dd = fetchWord();
				mem->write(dd, *rp);
				setT(20);
				LOG_OPCODE("LD (" << setw(4) << dd  << ")," << txt_rp1[shift_IXY][2]);
			} break;
			case 3: { // LD (nn),A
				uint16_t  dd = fetchWord();				
				mem->write(dd, reg.AF.A);				// Write the accumulator to memory
				setT(13);
				LOG_OPCODE("LD (" << setw(4) << dd << "),A");
			} break;
		}
	}
	else {
		switch(p) {
			case 0: { // LD A,(BC)
				reg.AF.A = mem->readByte(reg.BC.W);
				setT(7);
				LOG_OPCODE("LD A,(BC)");
			} break;
			case 1: { // LD A,(DE)
				reg.AF.A = mem->readByte(reg.DE.W);
				setT(7);
				LOG_OPCODE("LD A,(DE)");
			} break;
			case 2: { // LD HL/IX/IY,(nn)
				uint16_t* rp = lut_rp1[shift_IXY][2];
				uint16_t  dd = fetchWord();				
				*rp = mem->readWord(dd);
				setT(20);
				LOG_OPCODE("LD " << txt_rp1[shift_IXY][2] << ",(" << setw(4) << dd  << ")");
			} break;
			case 3: { // LD A,(nn)
				uint16_t  dd = fetchWord();				
				reg.AF.A = mem->readByte(dd);				// Read the accumulator from memory
				setT(13);
				LOG_OPCODE("LD A,(" << setw(4) << dd << ")");
			} break;
		}
	}
}

//
// X=0, Z=3: 16-bit increment/decrement
//
void Z80::execute_x0z3() {
	uint16_t* rp = lut_rp1[shift_IXY][p];
	if (q == 0) {	// INC
		(*rp)++;
		LOG_OPCODE("INC " << txt_rp1[shift_IXY][p]);
	}
	else {			// DEC
		(*rp)--;
		LOG_OPCODE("DEC " << txt_rp1[shift_IXY][p]);
	}
	setT(shift_IXY ? 10 : 6);
}

//
// X=0, Z=4: 8-bit increment
//
void Z80::execute_x0z4() {
	uint8_t* r = lut_r[shift_IXY][y];	// Pointer to the register memory or NULL if RAM
	uint8_t  c;
	uint8_t  b;
	if (r) {
		c = b = *r;
		*r = ++b;
		setT(4);
	}
	else {
		uint16_t a = getInd(shift_IXY);
		c = b = mem->readByte(a);
		mem->write(a, ++b);
		setT(shift_IXY ? 23 : 11);
	}
	LOG_OPCODE("INC " << txt_r[shift_IXY][y]);
	reg.setFlagsSZ(b);
	reg.setFlagsXY(b);
	reg.AF.B = (((c & 0x0F) + 1) & 0x10) != 0;
	reg.AF.P = (c == 0x7F);
	reg.AF.N = 0;
}

//
// X=0, Z=5: 8-bit decrement
//
void Z80::execute_x0z5() {
	uint8_t* r = lut_r[shift_IXY][y];	// Pointer to the register memory or NULL if RAM
	uint8_t  c;
	uint8_t  b;
	if (r) {
		c = b = *r;
		*r = --b;
		setT(4);
	}
	else {
		uint16_t a = getInd(shift_IXY);
		c = b = mem->readByte(a);
		mem->write(a, --b);
		setT(shift_IXY ? 23 : 11);
	}
	LOG_OPCODE("DEC " << txt_r[shift_IXY][y]);
	reg.setFlagsSZ(b);
	reg.setFlagsXY(b);
	reg.AF.B = (((c & 0x0F) - 1) & 0x10) != 0;
	reg.AF.P = (c == 0x80);
	reg.AF.N = 1;
}

//
// X=0, Z=6: 8-bit load immediate
//
void Z80::execute_x0z6() {
	uint8_t* r = lut_r[shift_IXY][y];
	if (r) {							// If it is a register
		fetch();						// Fetch the immediate value
		*r = data;						// And store
		setT(7);
	}
	else {
		uint16_t a = getInd(shift_IXY);	// Otherwise next byte is the index
		fetch();						// Followed by the immediate value
		mem->write(a, data);			// And store
		setT(shift_IXY ? 19 : 7);
	}
	LOG_OPCODE("LD " << txt_r[shift_IXY][y] << "," << setw(2) << (uint16_t)data);
}

//
// X=0, Z=7: Assorted operations on accumulator flags
//
void Z80::execute_x0z7() {
	alu2_t f = lut_alu2[y];				// Look up the ALU function
	(reg.*f)();							// And run
	setT(y == 4 ? 8 : 4);				// DAA is 8 T-states, all other opcodes are 4
	LOG_OPCODE(txt_alu2[y]);
}

// 8 bit loading
//
void Z80::execute_x1__()
{
	if (y == 6 && z == 6) {	// HALT
		halted = true;
		setT(4);
		LOG_OPCODE("HALT");
	}
	else {					// LD ry,rz
		uint8_t  ss = (z != 6 && y == 6 ? 0 : shift_IXY);
		uint8_t  sd = (y != 6 && z == 6 ? 0 : shift_IXY);

		uint8_t* rs = lut_r[ss][z];					// The source
		uint8_t* rd = lut_r[sd][y];					// The destination (cannot be IXL/H)
		
		if (rd) {									// Destination is a register
			if (rs) {								// Source is a register
				*rd = *rs;							// Copy the value to the destination
				setT(4);
			}
			else {									// Source is memory
				*rd = mem->readByte(getInd(shift_IXY));
				setT(shift_IXY ? 19 : 7);
			}
		}
		else {										// Destination is memory
			if (rs) {								// Source is register
				mem->write(getInd(shift_IXY), *rs);	// Copy the register to the memory
				setT(shift_IXY ? 19 : 7);
			}
			else {
				throw runtime_error("execute_x1: source and data are both memory locations");
			}
		}
		LOG_OPCODE("LD " << txt_r[sd][y] << "," << txt_r[ss][z]);
	}
}

// Operations on accumulator and register/memory location
//
void Z80::execute_x2__()
{
	uint8_t* r = lut_r[shift_IXY][z];			// Pointer to the register or HL
	alu1_t f = lut_alu1[y];						// Look up the ALU function
	if (r) {									// If it's a register then
		(reg.*f)(*r);							// Use the registry contents
		setT(4);
	}
	else {										// Otherwise do it on a memory location
		(reg.*f)(mem->readByte(getInd(shift_IXY)));
		setT(shift_IXY ? 19 : 7);
	}
	LOG_OPCODE(txt_alu1[y] << " " << txt_r[shift_IXY][z]);
}

//
// X=3, Z=0: Conditional return
//
void Z80::execute_x3z0() {
	cc_t f = lut_cc[y];		// Look up the cc function
	bool c = (reg.*f)();	// Get the condition
	if(c) {
		reg.PC = pop();
		setT(11);
	}
	else {
		setT(5);
	}
	LOG_OPCODE("RET " << txt_cc[y]);
}

//
// X=3, Z=1: POP and various operations
//
void Z80::execute_x3z1()
{
	if (q == 0) {	// POP
		uint16_t* rp = lut_rp2[shift_IXY][p];
		*rp = pop();
		setT(shift_IXY ? 14 : 10);
		LOG_OPCODE("POP " << txt_rp2[shift_IXY][p]);
	}
	else {
		switch (p) {
			case 0: { // RET
				reg.PC = pop();
				setT(10);
				LOG_OPCODE("RET");
			} break;
			case 1: { // EXX
				reg.exx();
				setT(4);
				LOG_OPCODE("EXX");
			} break;
			case 2: { // JP (HL/IX/IY)
				uint16_t* rp = lut_rp1[shift_IXY][2];
				reg.PC = *rp;
				setT(shift_IXY ? 8 : 4);
				LOG_OPCODE("JP (" << txt_rp1[shift_IXY][2] << ")");
			} break;
			case 3: { // LD SP,HL/IX/IY
				uint16_t* rp = lut_rp1[shift_IXY][2];
				reg.SP = *rp;
				setT(shift_IXY ? 10: 6);
				LOG_OPCODE("LD SP," << txt_rp1[shift_IXY][2]);
			} break;
		}
	}
}

//
// X=3, Z=2: Conditional jump
//
void Z80::execute_x3z2() {
	cc_t f = lut_cc[y];			// Look up the cc function
	bool c = (reg.*f)();		// Get the condition
	uint16_t dd = fetchWord();	// And the address
	if(c) {
		reg.PC = dd;
	}
	setT(10);
	LOG_OPCODE("JP " << txt_cc[y] << "," << setw(4) << dd);
}

//
// X=3, Z=3: Assorted operations
//
void Z80::execute_x3z3() {
	switch(y) {
		case 0: { // JP
			reg.PC = fetchWord();
			setT(10);
			LOG_OPCODE("JP " << setw(4) << reg.PC);
		} break;
		case 1: { // CB prefix
			throw runtime_error("execute_x3z3: invalid operation");
		} break;
		case 2: { // OUT (n),A
			fetch();
			ports->out((reg.AF.A << 8) | data, reg.AF.A);
			setT(11);
			LOG_OPCODE("OUT (" << setw(2) << (uint16_t)data << "),A");
		} break;
		case 3: { // IN A,(n)
			fetch();
			reg.AF.A = ports->in((reg.AF.A << 8) | data);
			setT(11);
			LOG_OPCODE("IN A,(" << setw(2) << (uint16_t)data << ")");
		} break;
		case 4: { // EX (SP),rp
			uint16_t* rp = lut_rp1[shift_IXY][2];
			uint16_t  dd = mem->readWord(reg.SP); 	// Read the value from the stack
			mem->write(reg.SP, *rp);				// Write the register to the stack
			*rp = dd;								// Set the register to the new value
			setT(shift_IXY ? 23 : 19);
			LOG_OPCODE("EX (SP)," << txt_rp1[shift_IXY][2]);
		} break;
		case 5: { // EX DE,HL
			reg.exdehl();
			setT(4);
			LOG_OPCODE("EX DE,HL");
		} break;
		case 6: { // DI
			reg.IFF1 = reg.IFF2 = false;
			setT(4);
			LOG_OPCODE("DI");
		} break;
		case 7: { // EI
			reg.IFF1 = reg.IFF2 = true;
			setT(4);
			LOG_OPCODE("EI");
		} break;
	}
}

//
// X=3, Z=4: Conditional call
//
void Z80::execute_x3z4() {
	cc_t f = lut_cc[y];			// Look up the cc function
	bool c = (reg.*f)();		// Get the condition
	uint16_t dd = fetchWord();	// And the address
	if(c) {
		push(reg.PC);
		reg.PC = dd;
		setT(17);
	}
	else {
		setT(10);
	}
	LOG_OPCODE("CALL " << txt_cc[y] << "," << setw(4) << dd);
}

//
// X=3, Z=5: PUSH and various operations
//
void Z80::execute_x3z5()
{
	if (q == 0) {		// PUSH
		uint16_t* rp = lut_rp2[shift_IXY][p];
		push(*rp);
		setT(shift_IXY ? 15 : 11);
		LOG_OPCODE("PUSH " << txt_rp2[shift_IXY][p]);
	}
	else {
		if(p == 0) {	// CALL nn
			uint16_t dd = fetchWord();
			push(reg.PC);
			reg.PC = dd;
			setT(17);
			LOG_OPCODE("CALL " << setw(4) << dd);
		}
		else {
			throw runtime_error("execute_x3z5: invalid operation");
		}
	}
}

//
// X=3, Z=6: Operate on accumulator and immediate operand
//
void Z80::execute_x3z6() {
	uint8_t* r = &reg.AF.A;		// Pointer to the accumulator
	fetch();					// Fetch the immediate operand
	alu1_t f = lut_alu1[y];		// Look up the ALU function
	if (f) {
		(reg.*f)(data);			// And execute it
		setT(7);
	}
	LOG_OPCODE(txt_alu1[y] << " " << setw(2) << (uint16_t)data);
}

//
// X=3, Z=7: Restart instructions
//
void Z80::execute_x3z7() {
	push(reg.PC);
	uint16_t a = y * 8;
	reg.PC = a; 
	setT(11);
	LOG_OPCODE("RST " << setw(2) << a);
}

// Push v on the stack
//
void Z80::push(uint16_t v)
{
	uint8_t lsb = v & 0xFF;
	uint8_t msb = v >> 8;
	mem->write(--reg.SP, msb);
	mem->write(--reg.SP, lsb);
}

// Pop off the stack
//
uint16_t Z80::pop()
{
	return mem->readByte(reg.SP++) | mem->readByte(reg.SP++) << 8;	// Pop LSB then MSB
}

// Get an indirect pointer from (HL), (IX + d) or (IY + d)
//
uint16_t Z80::getInd(uint8_t s) {
	if(s == 0) {					// shift_IXY is 0, so 
		return reg.HL.W;			// just get RAM pointer to (HL)
	}
	fetch();						// Otherwise get the index
	return getIXY(s, data);			// And get the RAM pointer to (IX/Y + d)
}

// Get an indirect pointer from IX+d or IY+d
//
uint16_t Z80::getIXY(uint8_t s, uint8_t d) {
	int8_t disp = (int8_t)d;
	switch(s) {
		case 1: return reg.IX.W + disp;	// (IX + d)
		case 2: return reg.IY.W + disp;	// (IY + d);
	}
	throw runtime_error("getIXY: invalid shift value");
}

void Z80::ldi() {	
	uint8_t b = mem->readByte(reg.HL.W++);
	mem->write(reg.DE.W++, b);
	reg.BC.W--;
	reg.AF.P = (reg.BC.W != 0);
	reg.AF.B = 0;
	reg.AF.N = 0;
	b+=reg.AF.A;
	reg.AF.X = !!(b & 0b00001000);
	reg.AF.Y = !!(b & 0b00000010);

}

void Z80::cpi() {	
	uint8_t d = mem->readByte(reg.HL.W++);
	uint8_t a = reg.AF.A;
	uint8_t b = a - d;
	reg.BC.W--;	
	reg.AF.S = (b > 0x7F);
	reg.AF.Z = (b == 0x00);
	reg.AF.B = (((a & 0x0F) - (d & 0x0F)) & 0x10) != 0;
	reg.AF.P = (reg.BC.W != 0);
	reg.AF.N = 1;
	b-=reg.AF.B;
	reg.AF.X = !!(b & 0b00001000);
	reg.AF.Y = !!(b & 0b00000010);
}

void Z80::ini() {	
	mem->write(reg.HL.W++, ports->in(reg.BC.W));
	reg.BC.H--;
	reg.AF.Z = (reg.BC.H == 0);
	reg.AF.N = 1;
}

void Z80::outi() {	
	ports->out(reg.BC.W, mem->readByte(reg.HL.W++));
	reg.BC.H--;
	reg.AF.Z = (reg.BC.H == 0);
	reg.AF.N = 1;
}

void Z80::ldd() {	
	uint8_t b = mem->readByte(reg.HL.W--);
	mem->write(reg.DE.W--, b);
	reg.BC.W--;
	reg.AF.P = (reg.BC.W != 0);
	reg.AF.B = 0;
	reg.AF.N = 0;	
	b+=reg.AF.A;
	reg.AF.X = !!(b & 0b00001000);
	reg.AF.Y = !!(b & 0b00000010);
}

void Z80::cpd() {	
	uint8_t d = mem->readByte(reg.HL.W--);
	uint8_t a = reg.AF.A;
	uint8_t b = a - d;
	reg.BC.W--;	
	reg.AF.S = (b > 0x7F);
	reg.AF.Z = (b == 0x00);
	reg.AF.B = (((a & 0x0F) - (d & 0x0F)) & 0x10) != 0;
	reg.AF.P = (reg.BC.W != 0);
	reg.AF.N = 1;
	b-=reg.AF.B;
	reg.AF.X = !!(b & 0b00001000);
	reg.AF.Y = !!(b & 0b00000010);
}

void Z80::ind() {	
	mem->write(reg.HL.W--, ports->in(reg.BC.W));
	reg.BC.H--;
	reg.AF.Z = (reg.BC.H == 0);
	reg.AF.N = 1;
}

void Z80::outd() {	
	ports->out(reg.BC.W, mem->readByte(reg.HL.W--));
	reg.BC.H--;
	reg.AF.Z = (reg.BC.H == 0);
	reg.AF.N = 1;
}

void Z80::ldir() {	
	ldi();
	if(reg.BC.W != 0) {
		reg.PC-=2;
	}
	else {
		blockop = false;
	}
}

void Z80::cpir() {	
	cpi();
	if(reg.BC.W != 0 && reg.AF.Z == 0) {
		reg.PC-=2;
	}	
	else {
		blockop = false;
	}
}

void Z80::inir() {	
	ini();
	if(reg.BC.H != 0) {
		reg.PC-=2;
	}
	else {
		blockop = false;
	}
}

void Z80::otir() {	
	outi();
	if(reg.BC.H != 0) {
		reg.PC-=2;
	}
	else {
		blockop = false;
	}
}

void Z80::lddr() {	
	ldd();
	if(reg.BC.W != 0) {
		reg.PC-=2;
	}
	else {
		blockop = false;
	}
}

void Z80::cpdr() {	
	cpd();
	if(reg.BC.W != 0 && reg.AF.Z == 0) {
		reg.PC-=2;
	}	
	else {
		blockop = false;
	}
}

void Z80::indr() {	
	ind();
	if(reg.BC.H != 0) {
		reg.PC-=2;
	}
	else {
		blockop = false;
	}
}

void Z80::otdr() {	
	outd();
	if(reg.BC.H != 0) {
		reg.PC-=2;
	}
	else {
		blockop = false;
	}
}

