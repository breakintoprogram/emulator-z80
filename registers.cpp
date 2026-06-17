//
// Title:	        Z80 registers
// Description:		Z80 registers and ALU emulation
// Author:	        Dean Belfield
// Created:	        22/05/2026
// Last Updated:	28/05/2026
//
// Modinfo:

#include "registers.h"

Registers::Registers() :
	PC(0),
	IFF1(false),
	IFF2(false)
{
}

void Registers::ex(REG* rp1, REG* rp2)
{
	uint16_t d = (*rp1).W;
	(*rp1).W = (*rp2).W;
	(*rp2).W = d;
}

void Registers::exx() {
	ex(&HL, &HL_);
	ex(&DE, &DE_);
	ex(&BC, &BC_);
}

void Registers::adda(uint8_t d) {
	uint8_t  a = AF.A;
	uint16_t w = a + d;
	uint8_t  b = w & 0xFF;
	AF.C = (w > 0xFF);
	AF.S = (b > 0x7F);
	AF.Z = (b == 0x00);
	AF.B = (((a & 0x0F) + (d & 0x0F)) & 0x10) != 0;
	AF.P = ((a ^ ~d) & (a ^ b) & 0x80) != 0;
	AF.N = 0;
	AF.A = b;
	setFlagsXY();
}

void Registers::adca(uint8_t d) {
	uint8_t  a = AF.A;
	uint16_t w = a + d + AF.C;
	uint8_t  b = w & 0xFF;
	uint8_t  c = AF.C;
	AF.C = (w > 0xFF);
	AF.S = (b > 0x7F);
	AF.Z = (b == 0x00);
	AF.B = (((a & 0x0F) + (d & 0x0F) + c) & 0x10) != 0;
	AF.P = ((a ^ ~d) & (a ^ b) & 0x80) != 0;
	AF.N = 0;
	AF.A = b;
	setFlagsXY();
}

void Registers::suba(uint8_t d) {
	uint8_t  a = AF.A;
	uint16_t w = a - d;
	uint8_t  b = w & 0xFF;
	AF.C = (w > 0xFF);
	AF.S = (b > 0x7F);
	AF.Z = (b == 0x00);
	AF.B = (((a & 0x0F) - (d & 0x0F)) & 0x10) != 0;
	AF.P = ((a ^ d) & (a ^ b) & 0x80) != 0;
	AF.N = 1;
	AF.A = b;
	setFlagsXY();
}

void Registers::sbca(uint8_t d) {
	uint8_t  a = AF.A;
	uint16_t w = a - d - AF.C;
	uint8_t  b = w & 0xFF;
	uint8_t  c = AF.C;
	AF.C = (w > 0xFF);
	AF.S = (b > 0x7F);
	AF.Z = (b == 0x00);
	AF.B = (((a & 0x0F) - (d & 0x0F) - c) & 0x10) != 0;
	AF.P = ((a ^ d) & (a ^ b) & 0x80) != 0;
	AF.N = 1;
	AF.A = b;
	setFlagsXY();
}

void Registers::cpa(uint8_t d) {
	uint8_t  a = AF.A;
	uint16_t w = a - d;
	uint8_t  b = w & 0xFF;
	AF.C = (w > 0xFF);
	AF.S = (b > 0x7F);
	AF.Z = (b == 0x00);
	AF.B = (((a & 0x0F) - (d & 0x0F)) & 0x10) != 0;
	AF.P = ((a ^ d) & (a ^ b) & 0x80) != 0;
	AF.N = 1;
	setFlagsXY(d);
}

void Registers::anda(uint8_t d) {
	AF.L = 0x10;	// Flags reset; Borrow flag set
	AF.A &= d;
	setFlagsSZ(AF.A);
	setFlagsP(AF.A);
	setFlagsXY();
}

void Registers::xora(uint8_t d) {
	AF.L = 0x00;	// Flags reset
	AF.A ^= d;
	setFlagsSZ(AF.A);
	setFlagsP(AF.A);
	setFlagsXY();
}

void Registers::ora(uint8_t d) {
	AF.L = 0x00;	// Flags reset
	AF.A |= d;
	setFlagsSZ(AF.A);
	setFlagsP(AF.A);
	setFlagsXY();
}

void Registers::neg() {
	uint8_t a = AF.A;
	AF.A = 0;
	suba(a);
}

void Registers::cpl() {
	AF.A=~AF.A;
	AF.B = 1;
	AF.N = 1;
	setFlagsXY();
}

void Registers::daa() {
	uint8_t a = AF.A;
	uint8_t b = AF.B;
	uint8_t c = AF.C;
	uint8_t i = 0;

	if (b || ((a & 0x0F) > 0x09)) {
		i |= 0x06;
	};

	if (c || (a > 0x9F) || ((a > 0x8F) && ((a & 0x0F) > 0x09))) {
		i |= 0x60;
	};
	
	if( a > 0x99) c = 1;
	
	if (AF.N) {
		suba(i);
	} else {
		adda(i);		
	};

	AF.C = c;
	setFlagsP(AF.A);
	setFlagsXY();
}

void Registers::scf() {
	AF.B = 0;
	AF.N = 0;
	AF.C = 1;
	setFlagsXY();
}

void Registers::ccf() {
	AF.B = AF.C;
	AF.N = 0;
	AF.C = !AF.C;
	setFlagsXY();
}

// Note that RLC A affects SZC, RLCA only affects carry
//
void Registers::rlc(uint8_t * r) {
	_rlc(r);
	setFlagsSZP(*r);
}
void Registers::rlca() {
	_rlc(&AF.A);
}
void Registers::_rlc(uint8_t * r) {
	uint8_t d = *r;				// The data to be operated on
	uint8_t c = (d&0x80)>>7;	// The bit to be shifted out is bit 7
	d = d << 1 | c;				// Shift left, and copy the bit shifted out into bit 0
	AF.C = c;					// Carry is set to the bit shifted out
	AF.B = 0;
	AF.N = 0;
	setFlagsXY(d);
	*r = d;						// Store result back
}

// Note that RRC A affects SZC, RRCA only affects carry
//
void Registers::rrc(uint8_t * r) {
	_rrc(r);
	setFlagsSZP(*r);
}
void Registers::rrca() {
	_rrc(&AF.A);
}
void Registers::_rrc(uint8_t * r) {
	uint8_t d = *r;				// The data to be operated on
	uint8_t c = d&0x01;			// The bit to be shifted out is bit 0
	d = d >>1 | c<<7;			// Shift right, and copy the bit shifted out into bit 7
	AF.C = c;					// Carry is set to the bit shifted out
	AF.B = 0;
	AF.N = 0;
	setFlagsXY(d);
	*r = d;						// Store result back
}

// Note that RL A affects SZC, RLA only affects carry
//
void Registers::rl(uint8_t * r) {
	_rl(r);
	setFlagsSZP(*r);
}
void Registers::rla() {
	_rl(&AF.A);
}
void Registers::_rl(uint8_t * r) {
	uint8_t d = *r;				// The data to be operated on
	uint8_t c = (d&0x80)>>7;	// The bit to be shifted out is bit 7
	d = d << 1 | AF.C;			// Shift left, and copy carry into bit 0
	AF.C = c;					// Carry is set to the bit shifted out
	AF.B = 0;
	AF.N = 0;
	setFlagsXY(d);
	*r = d;						// Store result back
}

// Note that RR A affects SZC, RRA only affects carry
//
void Registers::rr(uint8_t * r) {
	_rr(r);
	setFlagsSZP(*r);
}
void Registers::rra() {
	_rr(&AF.A);
}
void Registers::_rr(uint8_t * r) {
	uint8_t d = *r;				// The data to be operated on
	uint8_t c = d&0x01;			// The bit to be shifted out is bit 0
	d = d >>1 | AF.C<<7; 		// Shift right, and copy carry into bit 7
	AF.C = c;					// Carry is set to the bit shifted out
	AF.B = 0;
	AF.N = 0;
	setFlagsXY(d);
	*r = d;						// Store result back
}

void Registers::sla(uint8_t * r) {
	uint8_t d = *r;				// The data to be operated on
	uint8_t c = (d&0x80)>>7;	// The bit to be shifted out is bit 7
	d = d << 1;					// Shift the data left; 0 is shifted in
	AF.C = c;					// Carry is set to the bit shifted out
	AF.B = 0;
	AF.N = 0;
	setFlagsSZP(d);
	setFlagsXY(d);
	*r = d;						// Store result back
}

void Registers::sra(uint8_t * r) {
	uint8_t d = *r;				// The data to be operated on
	uint8_t s = d&0x80;			// The sign bit we want to preserve
	uint8_t c = d&0x01;			// The bit to be shifted out is bit 0
	d = d >> 1 | s;				// Shift the data right, preserving bit 7
	AF.C = c;					// Carry flag is set to the bit shifted out
	AF.B = 0;
	AF.N = 0;
	setFlagsSZP(d);
	setFlagsXY(d);
	*r = d;						// Store result back
}

void Registers::sll(uint8_t * r) {
	uint8_t d = *r;				// The data to be operated on
	uint8_t c = (d&0x80)>>7;	// The bit to be shifted out is bit 7
	d = d << 1 | 1;				// Shift the data left, setting bit 0 to 1
	AF.C = c;					// Carry flag is set to the bit shifted out
	AF.B = 0;
	AF.N = 0;
	setFlagsSZP(d);
	setFlagsXY(d);
	*r = d;						// Store the result back
}

void Registers::srl(uint8_t * r) {
	uint8_t d = *r;				// The data to be operated on
	uint8_t c = d&0x01;			// The bit to be shifted out is bit 0
	d = d >>1;					// Shift the data right, setting bit 7 to 0
	AF.C = c;					// Carry flag is set to the bit shifted out
	AF.B= 0;
	AF.N = 0;
	setFlagsSZP(d);
	setFlagsXY(d);
	*r = d;						// Store the result back
}

void Registers::incR(uint8_t d) {
	R = ((R + d) & 0x7F | (R & 0x80));
}

void Registers::setFlagsSZ(uint8_t d) {
	AF.S = (d >= 0x80);
	AF.Z = (d == 0x00);
}	

void Registers::setFlagsP(uint8_t d) {
	AF.P = lut_parity[d];
}	

void Registers::setFlagsSZP(uint8_t d) {
	setFlagsSZ(d);
	setFlagsP(d);
}

void Registers::setFlagsXY() {
	setFlagsXY(AF.A);
}

void Registers::setFlagsXY(uint8_t d) {
	AF.L &= 0b11010111;
	AF.L |= d & 0b00101000;
}

bool Registers::F_NZ() { return AF.Z == 0; }
bool Registers::F_Z()  { return AF.Z == 1; }
bool Registers::F_NC() { return AF.C == 0; }
bool Registers::F_C()  { return AF.C == 1; }
bool Registers::F_PO() { return AF.P == 0; }
bool Registers::F_PE() { return AF.P == 1; }
bool Registers::F_P()  { return AF.S == 0; }
bool Registers::F_M()  { return AF.S == 1; }