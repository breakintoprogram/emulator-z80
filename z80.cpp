//
// Title:	        Z80 CPU
// Description:		Z80 CPU emulation
// Author:	        Dean Belfield
// Created:	        22/05/2026
// Last Updated:	28/05/2026
//
// Modinfo:

#include "z80.h"

Z80::Z80(uint8_t* ram, out_t pout, in_t pin) :
	ram(ram),
	pout(pout),
	pin(pin),
	p(0),
	q(0),
	x(0),
	y(0),
	z(0),
	shift_EXT(0),
	shift_IXY(0),
	data(0),
	breakpoints(),
	callDepth(0),
	interrupt(0),
	singleStep(false),
	cycle(0),
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

void Z80::setTrace(bool value) {
	trace = value;
}

uint16_t Z80::getCycle() {
	return cycle;
}

// Do the NMI
//
void Z80::interruptRequest(uint8_t i) {
	if(reg.IFF1 && reg.IFF2) {	// If the interrupts are enabled
		interrupt = i;
	}
}

// Reset the CPU
//
void Z80::reset()
{
	reg.PC = 0;
}

// Output some debugging preamble
//
void Z80::debug() {
	if (shift_EXT == 0 && shift_IXY == 0) {
		if (trace) {
			cout << setfill('0') << hex;
			cout << "F=[";
			cout << (reg.AF.S  ? 'S' : '-');
			cout << (reg.AF.Z  ? 'Z' : '-');
			cout << (reg.AF.F5 ? '5' : '-');
			cout << (reg.AF.H  ? 'H' : '-');
			cout << (reg.AF.F3 ? '3' : '-');
			cout << (reg.AF.P  ? 'P' : '-');
			cout << (reg.AF.N  ? 'N' : '-');
			cout << (reg.AF.C  ? 'C' : '-');
			cout << "] ";
			cout << "A=" << setw(2) <<(uint16_t)reg.AF.A << " ";
			cout << "BC=" << setw(4) << reg.BC.W << " ";
			cout << "DE=" << setw(4) << reg.DE.W << " ";
			cout << "HL=" << setw(4) << reg.HL.W << " ";
			cout << "IX=" << setw(4) << reg.IX.W << " ";
			cout << "IY=" << setw(4) << reg.IY.W << " ";
			cout << "PC=" << setw(4) << reg.PC << " : ";
		}
		if(find(breakpoints.begin(), breakpoints.end(), reg.PC) != breakpoints.end()) {
			setSingleStep(true);
		}
	}
}

// Fetch an opcode
//
void Z80::fetch()
{
	data = readByte(reg.PC++);
	if (trace) {
		cout << setfill('0') << setw(2) << hex << (uint16_t)data << " ";
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
	switch(shift_EXT) {
		case 0xCB: execute_CB(); break;
		case 0xED: execute_ED(); break;
		default: {
			auto f = lut_xz[x][z];
			(this->*f)();
		}
	}
	cycle++;
	//
	// Might be a bit of a bodge for the moment but only handle interrupts
	// when the shift registers are both 0, i.e. finished processing last instruction
	//
	if(shift_EXT == 0 && shift_IXY == 0) {
		cycle = 0;
		reg.R++;
		if(interrupt > 0) {		// If an interrupt has been requested
			interrupt = 0;
			reg.IFF1 = false;	// Disable the interrupts
			reg.IFF2 = false;
			push(reg.PC);		// Push the current program counter on the stack
			reg.PC = 0x0038;	// Set the program counter to the maskable interrupt routine
			callDepth++;		// Increment the call depth for debugging purposes
		}
		if (trace) cout << endl;
	}
}

void Z80::execute_CB() {
	//
	// Special case for DD/FD prefixes
	// The index has been stored in index_CB
	// This will have immediately followed the DDCB or FDCB shift pair
	//
	if (shift_IXY != 0 ) {		
		uint8_t* p = getIXYPtr(shift_IXY, index_CB);		
		uint8_t  s = 1<<y;	
		switch(x) {
			case 0: { // ROT
				if(!isROM(p)) {
					auto f = lut_rot[y];
					(this->*f)(p);
					reg.AF.B = 0;
					reg.AF.N = 0;
				}		 
			} break;
			case 1: { // BIT
				reg.AF.Z = (((*p) & s) == 0);
			} break;
			case 2: { // RES
				if(!isROM(p)) {
					(*p)&=~s;
				}
			} break;
			case 3: { // SET
				if(!isROM(p)) {
					(*p)|=s;
				}				 
			} break;
		}
		shift_IXY = 0;
	}
	//
	// Normal CB operations
	//
	else {
	 	uint8_t* p = t_r[0][z];				// Look up the register; NULL if (HL)
		uint8_t  s = 1<<y;		
		switch(x) {
			case 0: { // ROT
				auto f = lut_rot[y];		// Look up the ROT operation
				if (p) {					// If it is a register then
					(this->*f)(p);			// Execute the function on the register
				}
				else {
					p = getIndPtr(0);		// p should now be a memory pointer
					if(!isROM(p)) {			// If it is not in ROM then
						(this->*f)(p);		// Execute the function on the register						
					}
				}
			} break;
			case 1: { // BIT y,r[z]			// This is a read only operation so no need for ROM check
				if (!p) {					// If it is not a register then
					p = getIndPtr(0);		// Get the memory address
				}
				reg.AF.Z = (((*p) & s) == 0);
			} break;
			case 2: { // RES y,r[z]
				if (p) {
					(*p)&=~s;				// p is a register pointer
				}
				else {
					p = getIndPtr(0);		// p should now be a memory pointer
					if(!isROM(p)) {			// If it is not in ROM then
						(*p)&=~s;			// Execute the function on the memory location						
					}					
				}
			} break;
			case 3: { // SET y,r[z]
				if (p) {
					(*p)|=s;				// p is a register pointer
				}
				else {
					p = getIndPtr(0);		// p should now be a memory pointer
					if(!isROM(p)) {			// If it is not in ROM then
						(*p)|=s;			// Execute the function on the memory location						
					}					
				}		
			} break;
		}
	}
	shift_EXT = 0;
}

void Z80::execute_ED() {
	switch(x) {

		case 1: {
			switch(z) {
				case 0: { // IN (C)
					if (y != 6) {
 						reg.AF.A = in(reg.BC.W);
					}
					reg.setFlagsSZ(reg.AF.A);
					reg.setFlagsP(reg.AF.A);
				} break;	
				case 1: { // OUT (C)
					if (y != 6) {
						out(reg.BC.W, reg.AF.A);
					}
				} break;
				case 2: { // ADC/SBC
					uint8_t c = reg.AF.C;
					uint16_t* rp1 = t_rp1[shift_IXY][2]; // HL, IX or IY
					uint16_t* rp2 = t_rp1[shift_IXY][p]; // The other register pair
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
						reg.AF.P = ((*rp1 ^ *rp2) & (*rp1 ^ w) & 0x8000) != 0;
						reg.AF.N = 0;
					}
					reg.AF.Z = (w == 0);
					reg.AF.C = (l > 0xFFFF);
					reg.AF.S = (w > 0x7FFF);
					*rp1 = w;
				} break;
				case 3: { // Load register pair from/to immediate address
					uint16_t* rp = t_rp1[shift_IXY][p];
					uint16_t  dd = fetchWord();
					if (q ==0) {
						writeWord(dd, *rp);	
					}
					else {
						*rp = readWord(dd);	
					}
				} break;
				case 4: { // NEG
					reg.A_neg();
				} break;
				case 5: { // RETI/RETN
					reg.PC = pop();
					callDepth--;
				} break;
				case 6: { // IM

				} break;
				case 7: { // Assorted ops
					switch(y) {
						case 0: { // LD I,A
							reg.I = reg.AF.A;
						} break;
						case 1: { // LD R,A
							reg.R = reg.AF.A;
						} break;
						case 2: { // LD A,I
							reg.AF.A = reg.I;
						} break;
						case 3: { // LD A,R
							reg.AF.A = reg.R;
						} break;
						case 4: { // RRD
							uint16_t d = readByte(reg.HL.W);
							uint8_t a = reg.AF.A;
							reg.AF.A = (reg.AF.A & 0xF0) | (d & 0x0F);
							d = ((a & 0x0F) << 4) | ((d & 0xF0) >> 4);
							writeByte(reg.HL.W, d);
							reg.setFlagsSZ(reg.AF.A);
							reg.setFlagsP(reg.AF.A);
							reg.AF.B = 0;
							reg.AF.N = 0;
						} break;
						case 5: { // RLD
							uint16_t d = readByte(reg.HL.W);
							uint8_t a = reg.AF.A;
							reg.AF.A = (reg.AF.A & 0xF0) | ((d & 0xF0) >> 4);
							d = ((d & 0x0F) << 4) | (a & 0x0F);
							writeByte(reg.HL.W, d);
							reg.setFlagsSZ(reg.AF.A);
							reg.setFlagsP(reg.AF.A);
							reg.AF.B = 0;
							reg.AF.N = 0;
						} break;
					}
				} break;
			}
		} break;

		case 2: { // Block instructions
			if(y > 4) {
				auto f = lut_bli[y-4][z];
				(this->*f)();
			}	
		} break;

		default: {
			execute_trap();
		} break;
	}
	shift_EXT = 0;
	shift_IXY = 0;
}

// Note that RLC A affects SZC, RLCA only affects carry
//
void Z80::rlc(uint8_t * r) {
	rlca(r);
	reg.setFlagsSZ(*r);
	reg.setFlagsP(*r);
}
void Z80::rlca(uint8_t * r) {
	uint8_t d = *r;				// The data to be operated on
	uint8_t c = (d&0x80)>>7;	// The bit to be shifted out is bit 7
	d = d << 1 | c;				// Shift left, and copy the bit shifted out into bit 0
	reg.AF.C = c;				// Carry is set to the bit shifted out
	*r = d;						// Store result back
}

// Note that RRC A affects SZC, RRCA only affects carry
//
void Z80::rrc(uint8_t * r) {
	rrca(r);
	reg.setFlagsSZ(*r);
	reg.setFlagsP(*r);
}
void Z80::rrca(uint8_t * r) {
	uint8_t d = *r;				// The data to be operated on
	uint8_t c = d&0x01;			// The bit to be shifted out is bit 0
	d = d >>1 | c<<7;			// Shift right, and copy the bit shifted out into bit 7
	reg.AF.C = c;				// Carry is set to the bit shifted out
	*r = d;						// Store result back
}

// Note that RL A affects SZC, RLA only affects carry
//
void Z80::rl(uint8_t * r) {
	rla(r);
	reg.setFlagsSZ(*r);
	reg.setFlagsP(*r);
}
void Z80::rla(uint8_t * r) {
	uint8_t d = *r;				// The data to be operated on
	uint8_t c = (d&0x80)>>7;	// The bit to be shifted out is bit 7
	d = d << 1 | reg.AF.C;		// Shift left, and copy carry into bit 0
	reg.AF.C = c;				// Carry is set to the bit shifted out
	*r = d;						// Store result back
}

// Note that RR A affects SZC, RRA only affects carry
//
void Z80::rr(uint8_t * r) {
	rra(r);
	reg.setFlagsSZ(*r);
	reg.setFlagsP(*r);
}
void Z80::rra(uint8_t * r) {
	uint8_t d = *r;				// The data to be operated on
	uint8_t c = d&0x01;			// The bit to be shifted out is bit 0
	d = d >>1 | reg.AF.C<<7; 	// Shift right, and copy carry into bit 7
	reg.AF.C = c;				// Carry is set to the bit shifted out
	*r = d;						// Store result back
}

void Z80::sla(uint8_t * r) {
	uint8_t d = *r;				// The data to be operated on
	uint8_t c = (d&0x80)>>7;	// The bit to be shifted out is bit 7
	d = d << 1;					// Shift the data left; 0 is shifted in
	reg.AF.C = c;				// Carry is set to the bit shifted out
	reg.setFlagsSZ(d);
	reg.setFlagsP(d);
	*r = d;						// Store result back
}

void Z80::sra(uint8_t * r) {
	uint8_t d = *r;				// The data to be operated on
	uint8_t s = d&0x80;			// The sign bit we want to preserve
	uint8_t c = d&0x01;			// The bit to be shifted out is bit 0
	d = d >> 1 & s;				// Shift the data right, preserving bit 7
	reg.AF.C = c;				// Carry flag is set to the bit shifted out
	reg.setFlagsSZ(d);
	reg.setFlagsP(d);
	*r = d;						// Store result back
}

void Z80::sll(uint8_t * r) {
	uint8_t d = *r;				// The data to be operated on
	uint8_t c = (d&0x80)>>7;	// The bit to be shifted out is bit 7
	d = d << 1 | 1;				// Shift the data left, setting bit 0 to 1
	reg.AF.C = c;				// Carry flag is set to the bit shifted out
	reg.setFlagsSZ(d);
	reg.setFlagsP(d);
	*r = d;						// Store the result back
}

void Z80::srl(uint8_t * r) {
	uint8_t d = *r;				// The data to be operated on
	uint8_t c = d&0x01;			// The bit to be shifted out is bit 0
	d = d >>1;					// Shift the data right, setting bit 7 to 0
	reg.AF.C = c;				// Carry flag is set to the bit shifted out
	reg.setFlagsSZ(d);
	reg.setFlagsP(d);
	*r = d;						// Store the result back
}

void Z80::execute_trap()
{
	cout << "unimplemented opcode" << endl;
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
		case 0:	break;
		//
		// EX AF,AF'
		//
		case 1: {
			reg.ex(&reg.AF, &reg.AF_);
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
		} break;
		//
		// JR n
		//
		case 3: {
			fetch();
			reg.PC += int8_t(data);
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
			}
		} break;
	}
	shift_IXY = 0;
}

//
// X=0, Z=1: 16-bit load immediate/add
//
void Z80::execute_x0z1() {
	if (q == 0) {	// LD rr,n
		uint16_t* rp = t_rp1[shift_IXY][p];
		uint16_t  dd = fetchWord();				
		*rp = dd;
	}
	else {			// ADD HL,rr
		uint16_t* rp1 = t_rp1[shift_IXY][2]; // HL/IX/IY
		uint16_t* rp2 = t_rp1[shift_IXY][p]; // The other register pair
		uint32_t  l = *rp1 + *rp2;
		uint16_t  w = (l & 0xFFFF);
		reg.AF.C = (l > 0xFFFF);
		reg.AF.B = (((*rp1 & 0xFFF) + (*rp2 & 0xFFF)) & 0x1000) != 0;
		reg.AF.P = ((*rp1 ^ *rp2) & (*rp1 ^ w) & 0x8000) != 0;
		reg.AF.N = 1;
		*rp1 = w;
	}
	shift_IXY = 0;
}

//
// X=0, Z=2: Indirect load
//
void Z80::execute_x0z2() {
	if (q == 0) {
		switch(p) {
			case 0: { // LD (BC),A
				writeByte(reg.BC.W, reg.AF.A);
			} break;
			case 1: { // LD (DE),A
				writeByte(reg.DE.W, reg.AF.A);
			} break;
			case 2: { // LD (nn),HL/IX/IY 
				uint16_t* rp = t_rp1[shift_IXY][2];
				uint16_t  dd = fetchWord();
				writeWord(dd, *rp);
			} break;
			case 3: { // LD (nn),A
				uint16_t  dd = fetchWord();				
				writeByte(dd, reg.AF.A);				// Write the accumulator to memory
			} break;
		}
	}
	else {
		switch(p) {
			case 0: { // LD A,(BC)
				reg.AF.A = readByte(reg.BC.W);
			} break;
			case 1: { // LD A,(DE)
				reg.AF.A = readByte(reg.DE.W);
			} break;
			case 2: { // LD HL/IX/IY,(nn)
				uint16_t* rp = t_rp1[shift_IXY][2];
				uint16_t  dd = fetchWord();				
				*rp = readWord(dd);
			} break;
			case 3: { // LD A,(nn)
				uint16_t  dd = fetchWord();				
				reg.AF.A = readByte(dd);				// Read the accumulator from memory
			} break;
		}
	}
	shift_IXY = 0;
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
	shift_IXY = 0;
}

//
// X=0, Z=4: 8-bit increment
//
void Z80::execute_x0z4() {
	uint8_t* p = t_r[shift_IXY][y];	// Pointer to the register memory or NULL if RAM
	uint8_t  a;
	if (p) {						// If it is a register then
		a = *p;						// The current value
		(*p)++;						// Just increment it
	}
	else {
		p = getIndPtr(shift_IXY);	// Get the address to be affected
		a = *p;						// The current value
		writeByte(p, (*p) + 1);		// Increment it
	}
	reg.setFlagsSZ(*p);
	reg.AF.B = (((a & 0x0F) + 1) & 0x10) != 0;
	reg.AF.P = (a == 0x7F);
	reg.AF.N = 0;
	shift_IXY = 0;
}

//
// X=0, Z=5: 8-bit decrement
//
void Z80::execute_x0z5() {
	uint8_t* p = t_r[shift_IXY][y];	// Pointer to the register memory or NULL if RAM
	uint8_t  a;						// The current value
	if (p) {						// If it is a register then
		a = *p;						// The current value
		(*p)--;						// Just decrement it
	}
	else {
		p = getIndPtr(shift_IXY);	// Get the address to be affected
		a = *p;						// The current value
		writeByte(p, (*p) - 1);		// Decrement it
	}
	reg.setFlagsSZ(*p);
	reg.AF.B = (((a & 0x0F) - 1) & 0x10) != 0;
	reg.AF.P = (a == 0x80);
	reg.AF.N = 1;
	shift_IXY = 0;
}

//
// X=0, Z=6: 8-bit load immediate
//
void Z80::execute_x0z6() {
	uint8_t* p = t_r[shift_IXY][y];
	if (p) {						// If it is a register
		fetch();					// Fetch the immediate value
		*p = data;					// And store
	}
	else {
		p = getIndPtr(shift_IXY);	// Otherwise next byte is the index
		fetch();					// Followed by the immediate value
		writeByte(p, data);			// And store
	}
	shift_IXY = 0;
}

//
// X=0, Z=7: Assorted operations on accumulator flags
//
void Z80::execute_x0z7() {
	switch(y) {
		case 0: rlca(&reg.AF.A); break;			// RLCA
		case 1: rrca(&reg.AF.A); break;			// RRCA
		case 2: rla (&reg.AF.A); break;			// RLA
		case 3: rra (&reg.AF.A); break;			// RRA
		case 4: reg.A_daa(); break;				// DAA
		case 5: reg.A_not(); break;				// CPL
		case 6: reg.AF.C = 1; break;			// SCF
		case 7: reg.AF.C = !reg.AF.C; break;	// CCF
	}
	shift_IXY = 0;
}

// 8 bit loading
//
void Z80::execute_x1__()
{
	if (y == 6 && z == 6) {	// HALT
		if(!reg.IFF1) {
			reg.PC--;
		}
	}
	else {					// LD ry,rz
		uint8_t* pz = t_r[0][z];				// The source (cannot be IXL/H)
		if (pz == NULL) {						// If it is not a register then
			pz = getIndPtr(shift_IXY);			// Point to a memory location
		}

		uint8_t* py = t_r[0][y];				// The destination (cannot be IXL/H)
		if (py) {								// If it is a register then
			*py = *pz;							// Just copy it to the register
		}
		else {			
			py = getIndPtr(shift_IXY);			// Otherwise
			writeByte(py, *pz);					// Write to the memory location
		}
	}
	shift_IXY = 0;
}

// Operations on accumulator and register/memory location
//
void Z80::execute_x2__()
{
	uint8_t* p = t_r[shift_IXY][z];				// Pointer to the register or HL
	if (p == NULL) {
		p = getIndPtr(shift_IXY);
	}
	auto f = lut_alu[y];						// Look up the ALU function
	(reg.*f)(*p);								// And execute it
	shift_IXY = 0;
}

//
// X=3, Z=0: Conditional return
//
void Z80::execute_x3z0() {
	auto f = lut_cc[y];		// Look up the cc function
	bool c = (reg.*f)();	// Get the condition
	if(c) {
		reg.PC = pop();
		callDepth--;
	}
	shift_IXY = 0;
}

//
// X=3, Z=1: POP and various operations
//
void Z80::execute_x3z1()
{
	if (q == 0) {	// POP
		uint16_t* rp = t_rp2[shift_IXY][p];
		*rp = pop();
	}
	else {
		switch (p) {
			case 0: { // RET
				reg.PC = pop();
				callDepth--;
			} break;
			case 1: { // EXX
				reg.ex(&reg.HL, &reg.HL_);
				reg.ex(&reg.DE, &reg.DE_);
				reg.ex(&reg.BC, &reg.BC_);
			} break;
			case 2: { // JP (HL/IX/IY)
				uint16_t* rp = t_rp1[shift_IXY][2];
				reg.PC = *rp;
			} break;
			case 3: { // LD SP,HL/IX/IY
				uint16_t* rp = t_rp1[shift_IXY][2];
				reg.SP = *rp;
			} break;
		}
	}
	shift_IXY = 0;
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
	shift_IXY = 0;
}

//
// X=3, Z=3: Assorted operations
//
void Z80::execute_x3z3() {
	switch(y) {
		case 0: { // JP
			reg.PC = fetchWord();
		} break;
		case 1: { // CB prefix
			shift_EXT = 0xCB;
			if(shift_IXY) {			// If it is a DDCB or FDCB opcode shift then
				fetch();			// The next byte is the index
				index_CB = data;	// Store it here for later
			}
		} break;
		case 2: { // OUT (n),A
			fetch();
			out((reg.AF.A << 8) | data, reg.AF.A);
		} break;
		case 3: { // IN A,(n)
			fetch();
			reg.AF.A=in((reg.AF.A << 8) | data);
		} break;
		case 4: { // EX (SP),rp
			uint16_t* rp = t_rp1[shift_IXY][2];
			uint16_t  dd = readWord(reg.SP); 	// Read the value from the stack
			writeWord(reg.SP, *rp);				// Write the register to the stack
			*rp = dd;							// Set the register to the new value
		} break;
		case 5: { // EX DE,HL
			reg.ex(&reg.DE, &reg.HL);
		} break;
		case 6: { // DI
			reg.IFF1 = false;
			reg.IFF2 = false;
		} break;
		case 7: { // EI
			reg.IFF1 = true;
			reg.IFF2 = true;
		} break;
	}
	if(shift_EXT == 0) shift_IXY = 0;
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
		callDepth++;
	}
	shift_IXY = 0;
}

//
// X=3, Z=5: PUSH and various operations
//
void Z80::execute_x3z5()
{
	if (q == 0) {	// PUSH
		uint16_t* rp = t_rp2[shift_IXY][p];
		push(*rp);
		shift_IXY = 0;
	}
	else {
		shift_IXY = 0;
		switch (p) {
			case 0: { // CALL nn
				uint16_t dd = fetchWord();
				push(reg.PC);
				reg.PC = dd;
				callDepth++;
			} break;
			case 1: { // DD prefix
				shift_IXY = 1;
			} break;
			case 2: { // ED prefix
				shift_EXT = 0xED;
			} break;
			case 3: { // FD prefix
				shift_IXY = 2;
			} break;
		}
	}
}

//
// X=3, Z=6: Operate on accumulator and immediate operand
//
void Z80::execute_x3z6() {
	uint8_t* r = &reg.AF.H;		// Pointer to the accumulator
	fetch();					// Fetch the immediate operand
	auto f = lut_alu[y];		// Look up the ALU function
	if (f) {
		(reg.*f)(data);			// And execute it
	}
	shift_IXY = 0;
}

//
// X=3, Z=7: Restart instructions
//
void Z80::execute_x3z7() {
	push(reg.PC);
	reg.PC = y * 8;
	callDepth++;
	shift_IXY = 0;
}

// Return true if the memory is in ROM - assumes p is somewhere in ram buffer
//
bool Z80::isROM(uint8_t* p) {
	uint64_t a = p - ram;
	if(a >= RAM_SIZE) {	// Check if we've tried to access an invalid location
		NOP;			// Something has gone horribly wrong, should stop processing here
	}
	return a < 0x4000;
}

// Write byte to ram
//
void Z80::writeByte(uint16_t a, uint8_t d) { // Into ram[a]
	writeByte(&ram[a], d);
}
void Z80::writeByte(uint8_t* p, uint8_t d) { // Into *p; assumes p is somewhere in ram buffer
	if(!isROM(p)) {
		*p = d;
	}
}

// Read a byte from ram
//
uint8_t Z80::readByte(uint16_t a) {
	return ram[a];
}

// Write a word to ram
//
void Z80::writeWord(uint16_t a, uint16_t d) {
	writeByte(a, d & 0xFF);		// Write the lsb byte out
	writeByte(a + 1, d >> 8);	// Write the msb byte out
}

// Read a word from ram
//
uint16_t Z80::readWord(uint16_t a) {
	return readByte(a) | (readByte(a + 1) << 8);
}

// Push v on the stack
//
void Z80::push(uint16_t v)
{
	writeByte(--reg.SP, v >> 8);	// Push MSB
	writeByte(--reg.SP, v & 0xFF);	// Push LSB
}

// Pop off the stack
//
uint16_t Z80::pop()
{
	return readByte(reg.SP++) | readByte(reg.SP++) << 8;	// Pop LSB then MSB
}

// Get an indirect pointer from (HL), (IX + d) or (IY + d)
//
uint8_t* Z80::getIndPtr(uint8_t s) {
	if(s == 0) {					// shift_IXY is 0, so 
		return &ram[reg.HL.W];		// just get RAM pointer to (HL)
	}
	fetch();						// Otherwise get the index
	return getIXYPtr(s, data);		// And get the RAM pointer to (IX/Y + d)
}

// Get an indirect pointer from IX+d or IY+d
//
uint8_t* Z80::getIXYPtr(uint8_t s, uint8_t d) {
	int8_t disp = int8_t(d);
	switch(s) {
		case 1: return &ram[reg.IX.W + disp]; // (IX + d)
		case 2: return &ram[reg.IY.W + disp]; // (IY + d)
	}
	return NULL;
}

void Z80::out(uint16_t addr, uint8_t v) {
	if(pout) {
		pout(addr, v);
	}
}

uint8_t Z80::in(uint16_t addr) {
	if(pin) {
		return pin(addr);
	}
	return 0;
}

void Z80::ldi() {	
	writeByte(reg.DE.W++, readByte(reg.HL.W++));
	reg.BC.W--;
	reg.AF.P = reg.BC.W != 0;
}

void Z80::cpi() {	
	reg.A_cp(readByte(reg.HL.W++));
	reg.BC.W--;
	reg.AF.P = reg.BC.W != 0;
}

void Z80::ini() {	
	writeByte(reg.HL.W++, in(reg.BC.W));
	reg.BC.L--;
}

void Z80::outi() {	
	out(reg.BC.W, readByte(reg.HL.W++));
	reg.BC.L--;
}

void Z80::ldd() {	
	writeByte(reg.DE.W--, readByte(reg.HL.W--));
	reg.BC.W--;
	reg.AF.P = reg.BC.W != 0;
}

void Z80::cpd() {	
	reg.A_cp(readByte(reg.HL.W++));
	reg.BC.W--;
	reg.AF.P = reg.BC.W != 0;
}

void Z80::ind() {	
	writeByte(reg.HL.W--, in(reg.BC.W));
	reg.BC.L--;
}

void Z80::outd() {	
	out(reg.BC.W, readByte(reg.HL.W--));
	reg.BC.L--;
}

void Z80::ldir() {	
	do {
		ldi();
	} while (reg.BC.W != 0);
}

void Z80::cpir() {	
	do {
		cpi();
	} while (reg.BC.W != 0 && reg.AF.Z == 0);
}

void Z80::inir() {	
	do {
		ini();
	} while (reg.BC.L != 0);
}

void Z80::otir() {	
	do {
		outi();
	} while (reg.BC.L != 0);
}

void Z80::lddr() {	
	do {
		ldd();
	} while (reg.BC.W != 0);
}

void Z80::cpdr() {	
	do {
		cpd();
	} while (reg.BC.W != 0 && reg.AF.Z == 0);
}

void Z80::indr() {	
	do {
		ind();
	} while (reg.BC.L != 0);
}

void Z80::otdr() {	
	do {
		outd();
	} while (reg.BC.L != 0);
}

