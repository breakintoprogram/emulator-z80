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

void Registers::A_add(uint8_t d) {
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

}

void Registers::A_adc(uint8_t d) {
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
}

void Registers::A_sub(uint8_t d) {
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
}

void Registers::A_sbc(uint8_t d) {
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
}

void Registers::A_and(uint8_t d) {
	AF.L = 0x10;	// Flags reset; Borrow flag set
	AF.A &= d;
	setFlagsSZ(AF.A);
	setFlagsP(AF.A);
}

void Registers::A_xor(uint8_t d) {
	AF.L = 0x00;	// Flags reset
	AF.A ^= d;
	setFlagsSZ(AF.A);
	setFlagsP(AF.A);
}
void Registers::A_or(uint8_t d) {
	AF.L = 0x00;	// Flags reset
	AF.A |= d;
	setFlagsSZ(AF.A);
	setFlagsP(AF.A);
}

void Registers::A_cp(uint8_t d) {
	uint8_t  a = AF.A;
	uint16_t w = a - d;
	uint8_t  b = w & 0xFF;
	AF.C = (w > 0xFF);
	AF.S = (b > 0x7F);
	AF.Z = (b == 0x00);
	AF.B = (((a & 0x0F) - (d & 0x0F)) & 0x10) != 0;
	AF.P = ((a ^ d) & (a ^ b) & 0x80) != 0;
	AF.N = 1;
}

void Registers::A_neg() {
	uint8_t a = AF.A;
	AF.A = 0;
	A_sub(a);
}

void Registers::A_not() {
	AF.A=~AF.A;
	AF.B = 1;
	AF.N = 1;
}

void Registers::A_daa() {
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
		A_sub(i);
	} else {
		A_add(i);		
	};

	AF.C = c;
	setFlagsP(AF.A);
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

bool Registers::F_NZ() { return AF.Z == 0; }
bool Registers::F_Z()  { return AF.Z == 1; }
bool Registers::F_NC() { return AF.C == 0; }
bool Registers::F_C()  { return AF.C == 1; }
bool Registers::F_PO() { return AF.P == 0; }
bool Registers::F_PE() { return AF.P == 1; }
bool Registers::F_P()  { return AF.S == 0; }
bool Registers::F_M()  { return AF.S == 1; }