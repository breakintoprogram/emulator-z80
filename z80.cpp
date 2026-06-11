//
// Title:	        Z80 CPU
// Description:		Z80 CPU emulation
// Author:	        Dean Belfield
// Created:	        22/05/2026
// Last Updated:	28/05/2026
//
// Modinfo:

#include "z80.h"

Z80::Z80(Mem* mem, Ports* ports) :
	mem(mem),
	ports(ports),
	p(0),
	q(0),
	x(0),
	y(0),
	z(0),
	shift_IXY(0),
	data(0),
	breakpoints(),
	interrupt(0),
	halted(false),
	singleStep(false),
	trace(false),
	traceStream(cout)
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

uint16_t Z80::getT() {
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

// Run one CPU cycle
//
void Z80::run() {
	if(!halted) {
		setT(0);
		debug();
		fetch();
		while(data == 0xDD || data == 0xFD) {
			shift_IXY = ((data & 0b00100000) >> 5) + 1;
			fetch();
		}
		decode();
		execute();
		if(getT() == 0) {
			throw runtime_error("No T-states registered for this instruction");
		}
	}
	interrupts();
	if (trace) {
		traceStream << "(" << dec << (uint16_t)getT() << "T)" << endl;
	}
}

// Output some debugging preamble
//
void Z80::debug() {
	if (trace) {
		dump(traceStream);
	}
	if(find(breakpoints.begin(), breakpoints.end(), reg.PC) != breakpoints.end()) {
		setSingleStep(true);
	}
}

void Z80::dump(ostream& stream) {
	dump(stream, false);
}
void Z80::dump(ostream& stream, bool newline) {
	stream << setfill('0') << hex;
	stream << "F=[";
	stream << (reg.AF.S  ? 'S' : '-');
	stream << (reg.AF.Z  ? 'Z' : '-');
	stream << (reg.AF.F5 ? '5' : '-');
	stream << (reg.AF.B  ? 'H' : '-');
	stream << (reg.AF.F3 ? '3' : '-');
	stream << (reg.AF.P  ? 'P' : '-');
	stream << (reg.AF.N  ? 'N' : '-');
	stream << (reg.AF.C  ? 'C' : '-');
	stream << "] ";
	stream << "A="  << setw(2) << (uint16_t)reg.AF.A << " ";
	stream << "BC=" << setw(4) << reg.BC.W << " ";
	stream << "DE=" << setw(4) << reg.DE.W << " ";
	stream << "HL=" << setw(4) << reg.HL.W << " ";
	stream << "IX=" << setw(4) << reg.IX.W << " ";
	stream << "IY=" << setw(4) << reg.IY.W << " ";
	stream << "PC=" << setw(4) << reg.PC   << " ";
	stream << "SP=" << setw(4) << reg.SP;
	if(newline) {
		stream << endl;
	}
	else {
		stream << " : ";
	}
}

// Fetch an opcode
//
void Z80::fetch()
{
	data = mem->readByte(reg.PC++);
	if (trace) {
		traceStream << setfill('0') << setw(2) << hex << (uint16_t)data << " ";
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
	}
	else if (data == 0xED) {
		execute_ED();
	}
	else {
		auto f = lut_xz[x][z];
		(this->*f)();
	}
	shift_IXY = 0;
	reg.R++;
	reg.R&=0x7F;
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
		halted = false;						// CPU is no longer halted
		if (reg.IFF1) {						// Are interrupts enabled?
			reg.IFF1 = false;				// Disable interrupts
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
		uint16_t a = getIXY(shift_IXY, data);	// The next byte is the index
		fetch();								// Fetch the instruction
		decode();								// And decode
		uint8_t* r = t_r[0][z];	
		uint8_t  s = 1<<y;	
		uint8_t  b;
		switch(x) {
			case 0: { // ROT
				auto f = lut_rot[y];
				b = mem->readByte(a);
				(reg.*f)(&b);
				mem->write(a, b);
				if (r) *r = b;
				reg.AF.B = 0;
				reg.AF.N = 0;
				setT(23);
			} break;
			case 1: { // BIT
				b = mem->readByte(a) & s;
				reg.AF.Z = (b == 0);
				reg.AF.S = (y == 7 && b != 0);
				reg.AF.P = reg.AF.Z;
				reg.AF.B = 1;
				reg.AF.N = 0;		
				setT(20);			
			} break;
			case 2: { // RES
				b = mem->readByte(a) & ~s;
				mem->write(a, b);
				if (r) *r = b;
				setT(23);
			} break;
			case 3: { // SET
				b = mem->readByte(a) | s;
				mem->write(a, b);
				if (r) *r = b;	
				setT(23);	 
			} break;
		}
	}
	//
	// Normal CB operations
	//
	else {
		fetch();								// Fetch the instruction
		decode();								// And decode
	 	uint8_t*  r = t_r[0][z];				// Look up the register; NULL if (HL)
		uint8_t   s = 1<<y;
		uint8_t   b;
		switch(x) {
			case 0: { // ROT
				auto f = lut_rot[y];			// Look up the ROT operation
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
			} break;
			case 1: { // BIT y,r[z]				// This is a read only operation so no need for ROM check
				b = r ? *r : mem->readByte(getInd(0));
				b &= s;							// Mask it out
				reg.AF.Z = (b == 0);
				reg.AF.S = (y == 7 && b != 0);
				reg.AF.P = reg.AF.Z;
				reg.AF.B = 1;
				reg.AF.N = 0;		
				setT(r ? 8 : 12);		
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
					uint8_t* r = t_r[0][y];
					uint8_t b = ports->in(reg.BC.W);
					if (r) {
						*r = b;
					}
					reg.setFlagsSZP(b);
					reg.AF.B = 0;
					reg.AF.N = 0;
					setT(12);
				} break;	
				case 1: { // OUT (C)
					uint8_t* r = t_r[0][y];
					if (r) {
						ports->out(reg.BC.W, *r);
					}
					setT(12);
				} break;
				case 2: { // ADC/SBC
					uint8_t c = reg.AF.C;
					uint16_t* rp1 = t_rp1[0][2]; // HL
					uint16_t* rp2 = t_rp1[0][p]; // The other register pair
					uint32_t  l;
					uint16_t  w;
					if (q == 0) {
						l = ((*rp1) - (*rp2) - c);
						w = l & 0xFFFF;
						reg.AF.B = (((*rp1 & 0xFFF) - (*rp2 & 0xFFF) - c) & 0x1000) != 0;
						reg.AF.P = ((*rp1 ^ *rp2) & (*rp1 ^ w) & 0x8000) != 0;
						reg.AF.N = 1;

					}
					else {
						l = ((*rp1) + (*rp2) + c);
						w = l & 0xFFFF;
						reg.AF.B = (((*rp1 & 0xFFF) + (*rp2 & 0xFFF) + c) & 0x1000) != 0;
						reg.AF.P = ((*rp1 ^ ~*rp2) & (*rp1 ^ w) & 0x8000) != 0;
						reg.AF.N = 0;
					}
					reg.AF.Z = (w == 0);
					reg.AF.C = (l > 0xFFFF);
					reg.AF.S = (w > 0x7FFF);
					*rp1 = w;
					setT(15);
				} break;
				case 3: { // Load register pair from/to immediate address
					uint16_t* rp = t_rp1[0][p];
					uint16_t  dd = fetchWord();
					if (q ==0) {
						mem->write(dd, *rp);	
					}
					else {
						*rp = mem->readWord(dd);	
					}
					setT(p == 2 ? 16 : 20);
				} break;
				case 4: { // NEG
					reg.neg();
					setT(4);
				} break;
				case 5: { // RETI/RETN
					reg.PC = pop();
					if (y != 1) { // Check for RETN
						reg.IFF1 = reg.IFF2;
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
				} break;
				case 7: { // Assorted ops
					switch(y) {
						case 0: { // LD I,A
							reg.I = reg.AF.A;
							setT(9);
						} break;
						case 1: { // LD R,A
							reg.R = reg.AF.A;
							setT(9);
						} break;
						case 2: { // LD A,I
							reg.AF.A = reg.I;
							reg.setFlagsSZ(reg.AF.A);
							reg.AF.P = reg.IFF2;
							reg.AF.B = 0;
							reg.AF.N = 0;
							setT(9);
						} break;
						case 3: { // LD A,R
							reg.AF.A = reg.R;
							reg.setFlagsSZ(reg.AF.A);
							reg.AF.P = reg.IFF2;
							reg.AF.B = 0;
							reg.AF.N = 0;
							setT(9);
						} break;
						case 4: { // RRD
							uint8_t d = mem->readByte(reg.HL.W);
							uint8_t a = reg.AF.A;
							reg.AF.A = (reg.AF.A & 0xF0) | (d & 0x0F);
							d = ((a & 0x0F) << 4) | ((d & 0xF0) >> 4);
							mem->write(reg.HL.W, d);
							reg.setFlagsSZP(reg.AF.A);
							reg.AF.B = 0;
							reg.AF.N = 0;
							setT(18);
						} break;
						case 5: { // RLD
							uint8_t d = mem->readByte(reg.HL.W);
							uint8_t a = reg.AF.A;
							reg.AF.A = (reg.AF.A & 0xF0) | ((d & 0xF0) >> 4);
							d = ((d & 0x0F) << 4) | (a & 0x0F);
							mem->write(reg.HL.W, d);
							reg.setFlagsSZP(reg.AF.A);
							reg.AF.B = 0;
							reg.AF.N = 0;
							setT(18);
						} break;
					}
				} break;
			}
		} break;

		case 2: { // Block instructions
			if(z < 4 && y >= 4) {
				auto f = lut_bli[y-4][z];
				(this->*f)();
				setT(16);
			}	
			else {
				throw runtime_error("execute_ED: invalid block instruction");
			}
		} break;

		default: {
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
		} break;
		//
		// EX AF,AF'
		//
		case 1: {
			reg.ex(&reg.AF, &reg.AF_);
			setT(4);
			break;
		}
		//
		// DJNZ n
		//
		case 2: {
			fetch();
			reg.BC.H--;
			if (reg.BC.H != 0) {
				reg.PC += int8_t(data);
			}
			setT(13);
		} break;
		//
		// JR n
		//
		case 3: {
			fetch();
			reg.PC += int8_t(data);
			setT(12);
		} break;
		//
		// JR c,n
		//
		default: {
			auto f = lut_cc[y-4];		// Look up the cc function
			bool c = (reg.*f)();		// Get the condition
			fetch();					// Fetch the relative jump value
			if(c) {						// If the condition true then
				reg.PC += int8_t(data);	// JR to the location
				setT(12);
			}
			else {
				setT(7);
			}
		} break;
	}
}

//
// X=0, Z=1: 16-bit load immediate/add
//
void Z80::execute_x0z1() {
	if (q == 0) {	// LD rr,n
		uint16_t* rp = t_rp1[shift_IXY][p];
		uint16_t  dd = fetchWord();				
		*rp = dd;
		setT(shift_IXY ? 14 : 10);
	}
	else {			// ADD HL,rr
		uint16_t* rp1 = t_rp1[shift_IXY][2]; // HL/IX/IY
		uint16_t* rp2 = t_rp1[shift_IXY][p]; // The other register pair
		uint32_t  l = (*rp1) + (*rp2);
		uint16_t  w = (l & 0xFFFF);
		reg.AF.C = (l > 0xFFFF);
		reg.AF.B = (((*rp1 & 0xFFF) + (*rp2 & 0xFFF)) & 0x1000) != 0;
		reg.AF.N = 0;
		*rp1 = w;
		setT(shift_IXY ? 15 : 11);
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
			} break;
			case 1: { // LD (DE),A
				mem->write(reg.DE.W, reg.AF.A);
				setT(7);
			} break;
			case 2: { // LD (nn),HL/IX/IY 
				uint16_t* rp = t_rp1[shift_IXY][2];
				uint16_t  dd = fetchWord();
				mem->write(dd, *rp);
				setT(20);
			} break;
			case 3: { // LD (nn),A
				uint16_t  dd = fetchWord();				
				mem->write(dd, reg.AF.A);				// Write the accumulator to memory
				setT(13);
			} break;
		}
	}
	else {
		switch(p) {
			case 0: { // LD A,(BC)
				reg.AF.A = mem->readByte(reg.BC.W);
				setT(7);
			} break;
			case 1: { // LD A,(DE)
				reg.AF.A = mem->readByte(reg.DE.W);
				setT(7);
			} break;
			case 2: { // LD HL/IX/IY,(nn)
				uint16_t* rp = t_rp1[shift_IXY][2];
				uint16_t  dd = fetchWord();				
				*rp = mem->readWord(dd);
				setT(20);
			} break;
			case 3: { // LD A,(nn)
				uint16_t  dd = fetchWord();				
				reg.AF.A = mem->readByte(dd);				// Read the accumulator from memory
				setT(13);
			} break;
		}
	}
}

//
// X=0, Z=3: 16-bit increment/decrement
//
void Z80::execute_x0z3() {
	uint16_t* rp = t_rp1[shift_IXY][p];
	if (q == 0) {	// INC
		(*rp)++;
	}
	else {			// DEC
		(*rp)--;
	}
	setT(shift_IXY ? 10 : 6);
}

//
// X=0, Z=4: 8-bit increment
//
void Z80::execute_x0z4() {
	uint8_t* r = t_r[shift_IXY][y];	// Pointer to the register memory or NULL if RAM
	uint8_t  c;
	uint8_t  b;
	if (r) {
		b = *r;
		c = b;
		b++;
		*r = b;
		setT(4);
	}
	else {
		uint16_t a = getInd(shift_IXY);
		b = mem->readByte(a);
		c = b ;
		b++;
		mem->write(a, b);
		setT(shift_IXY ? 23 : 11);
	}
	reg.setFlagsSZ(b);
	reg.AF.B = (((c & 0x0F) + 1) & 0x10) != 0;
	reg.AF.P = (c == 0x7F);
	reg.AF.N = 0;
}

//
// X=0, Z=5: 8-bit decrement
//
void Z80::execute_x0z5() {
	uint8_t* r = t_r[shift_IXY][y];	// Pointer to the register memory or NULL if RAM
	uint8_t  c;
	uint8_t  b;
	if (r) {
		b = *r;
		c = b;
		b--;
		*r = b;
		setT(4);
	}
	else {
		uint16_t a = getInd(shift_IXY);
		b = mem->readByte(a);
		c = b ;
		b--;
		mem->write(a, b);
		setT(shift_IXY ? 23 : 11);
	}
	reg.setFlagsSZ(b);
	reg.AF.B = (((c & 0x0F) - 1) & 0x10) != 0;
	reg.AF.P = (c == 0x80);
	reg.AF.N = 1;
}

//
// X=0, Z=6: 8-bit load immediate
//
void Z80::execute_x0z6() {
	uint8_t* r = t_r[shift_IXY][y];
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
}

//
// X=0, Z=7: Assorted operations on accumulator flags
//
void Z80::execute_x0z7() {
	auto f = lut_alu2[y];				// Look up the ALU function
	(reg.*f)();							// And run
	setT(y == 4 ? 8 : 4);				// DAA is 8 T-states, all other opcodes are 4
}

// 8 bit loading
//
void Z80::execute_x1__()
{
	if (y == 6 && z == 6) {	// HALT
		halted = true;
		setT(4);
	}
	else {					// LD ry,rz
		uint8_t  ss = (z != 6 && y == 6 ? 0 : shift_IXY);
		uint8_t  sd = (y != 6 && z == 6 ? 0 : shift_IXY);

		uint8_t* rs = t_r[ss][z];					// The source
		uint8_t* rd = t_r[sd][y];					// The destination (cannot be IXL/H)
		
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
	}
}

// Operations on accumulator and register/memory location
//
void Z80::execute_x2__()
{
	uint8_t* r = t_r[shift_IXY][z];				// Pointer to the register or HL
	auto f = lut_alu1[y];						// Look up the ALU function
	if (r) {									// If it's a register then
		(reg.*f)(*r);							// Use the registry contents
		setT(4);
	}
	else {										// Otherwise do it on a memory location
		(reg.*f)(mem->readByte(getInd(shift_IXY)));
		setT(shift_IXY ? 19 : 7);
	}
}

//
// X=3, Z=0: Conditional return
//
void Z80::execute_x3z0() {
	auto f = lut_cc[y];		// Look up the cc function
	bool c = (reg.*f)();	// Get the condition
	if(c) {
		reg.PC = pop();
		setT(11);
	}
	else {
		setT(5);
	}
}

//
// X=3, Z=1: POP and various operations
//
void Z80::execute_x3z1()
{
	if (q == 0) {	// POP
		uint16_t* rp = t_rp2[shift_IXY][p];
		*rp = pop();
		setT(shift_IXY ? 14 : 10);
	}
	else {
		switch (p) {
			case 0: { // RET
				reg.PC = pop();
				setT(10);
			} break;
			case 1: { // EXX
				reg.exx();
				setT(4);
			} break;
			case 2: { // JP (HL/IX/IY)
				uint16_t* rp = t_rp1[shift_IXY][2];
				reg.PC = *rp;
				setT(shift_IXY ? 8 : 4);
			} break;
			case 3: { // LD SP,HL/IX/IY
				uint16_t* rp = t_rp1[shift_IXY][2];
				reg.SP = *rp;
				setT(shift_IXY ? 10: 6);
			} break;
		}
	}
}

//
// X=3, Z=2: Conditional jump
//
void Z80::execute_x3z2() {
	auto f = lut_cc[y];			// Look up the cc function
	bool c = (reg.*f)();		// Get the condition
	uint16_t dd = fetchWord();	// And the address
	if(c) {
		reg.PC = dd;
	}
	setT(10);
}

//
// X=3, Z=3: Assorted operations
//
void Z80::execute_x3z3() {
	switch(y) {
		case 0: { // JP
			reg.PC = fetchWord();
			setT(10);
		} break;
		case 1: { // CB prefix
			throw runtime_error("execute_x3z3: invalid operation");
		} break;
		case 2: { // OUT (n),A
			fetch();
			ports->out((reg.AF.A << 8) | data, reg.AF.A);
			setT(11);
		} break;
		case 3: { // IN A,(n)
			fetch();
			reg.AF.A = ports->in((reg.AF.A << 8) | data);
			setT(11);
		} break;
		case 4: { // EX (SP),rp
			uint16_t* rp = t_rp1[shift_IXY][2];
			uint16_t  dd = mem->readWord(reg.SP); 	// Read the value from the stack
			mem->write(reg.SP, *rp);				// Write the register to the stack
			*rp = dd;								// Set the register to the new value
			setT(shift_IXY ? 23 : 19);
		} break;
		case 5: { // EX DE,HL
			reg.ex(&reg.DE, &reg.HL);
			setT(4);
		} break;
		case 6: { // DI
			reg.IFF1 = false;
			reg.IFF2 = false;
			setT(4);
		} break;
		case 7: { // EI
			reg.IFF1 = true;
			reg.IFF2 = true;
			setT(4);
		} break;
	}
}

//
// X=3, Z=4: Conditional call
//
void Z80::execute_x3z4() {
	auto f = lut_cc[y];			// Look up the cc function
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
}

//
// X=3, Z=5: PUSH and various operations
//
void Z80::execute_x3z5()
{
	if (q == 0) {		// PUSH
		uint16_t* rp = t_rp2[shift_IXY][p];
		push(*rp);
		setT(shift_IXY ? 15 : 11);
	}
	else {
		if(p == 0) {	// CALL nn
			uint16_t dd = fetchWord();
			push(reg.PC);
			reg.PC = dd;
			setT(17);
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
	auto f = lut_alu1[y];		// Look up the ALU function
	if (f) {
		(reg.*f)(data);			// And execute it
		setT(7);
	}
}

//
// X=3, Z=7: Restart instructions
//
void Z80::execute_x3z7() {
	push(reg.PC);
	reg.PC = y * 8;
	setT(11);
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
	int8_t disp = int8_t(d);
	switch(s) {
		case 1: return reg.IX.W + disp;	// (IX + d)
		case 2: return reg.IY.W + disp;	// (IY + d);
	}
	throw runtime_error("getIXY: invalid shift value");
}

void Z80::ldi() {	
	mem->write(reg.DE.W++, mem->readByte(reg.HL.W++));
	reg.BC.W--;
	reg.AF.P = (reg.BC.W != 0);
	reg.AF.B = 0;
	reg.AF.N = 0;
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
	mem->write(reg.DE.W--, mem->readByte(reg.HL.W--));
	reg.BC.W--;
	reg.AF.P = (reg.BC.W != 0);
	reg.AF.B = 0;
	reg.AF.N = 0;	
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
}

void Z80::cpir() {	
	cpi();
	if(reg.BC.W != 0 && reg.AF.Z == 0) {
		reg.PC-=2;
	}	
}

void Z80::inir() {	
	ini();
	if(reg.BC.H != 0) {
		reg.PC-=2;
	}
}

void Z80::otir() {	
	outi();
	if(reg.BC.H != 0) {
		reg.PC-=2;
	}
}

void Z80::lddr() {	
	ldd();
	if(reg.BC.W != 0) {
		reg.PC-=2;
	}
}

void Z80::cpdr() {	
	cpd();
	if(reg.BC.W != 0 && reg.AF.Z == 0) {
		reg.PC-=2;
	}	
}

void Z80::indr() {	
	ind();
	if(reg.BC.H != 0) {
		reg.PC-=2;
	}
}

void Z80::otdr() {	
	outd();
	if(reg.BC.H != 0) {
		reg.PC-=2;
	}
}

