//
// Title:	        Z80 registers
// Description:		Z80 registers and ALU emulation
// Author:	        Dean Belfield
// Created:	        22/05/2026
// Last Updated:	28/05/2026
//
// Modinfo:

#pragma once

#include <cstdint>

#include "defines.h"

using namespace std;

class Registers {
public:
	Registers();

	REG AF, AF_;
	REG BC, BC_;
	REG DE, DE_;
	REG HL, HL_;

	REG IX;
	REG	IY;

	uint16_t PC;
	uint16_t SP;

	uint8_t I;
	uint8_t R;
	uint8_t IM;
	
	bool    IFF1;
	bool    IFF2;

	void ex(REG* rp1, REG* rp2);
	void exx();

	void adda(uint8_t d);
	void adca(uint8_t d);
	void suba(uint8_t d);
	void sbca(uint8_t d);
	void anda(uint8_t d);
	void xora(uint8_t d);
	void ora(uint8_t d);
	void cpa(uint8_t d);
	void neg();
	void cpl();
	void daa();
	void scf();
	void ccf();
	
	void rlc(uint8_t* r);
	void rrc(uint8_t* r);
	void rl(uint8_t* r);
	void rr(uint8_t* r);
	void sla(uint8_t* r);
	void sra(uint8_t* r);
	void sll(uint8_t* r);
	void srl(uint8_t* r);
	void rlca();
	void rrca();
	void rla();
	void rra();

	void setFlagsSZ(uint8_t d);				// Set flags SZ based upon D
	void setFlagsP(uint8_t d);				// Set flags P based upon D
	void setFlagsSZP(uint8_t d);			// Set flags SZP based upon D

	bool F_NZ();
	bool F_Z();
	bool F_NC();
	bool F_C();
	bool F_PO();
	bool F_PE();
	bool F_P();
	bool F_M();

private:
	void _rlc(uint8_t* r);
	void _rrc(uint8_t* r);
	void _rl(uint8_t* r);
	void _rr(uint8_t* r);

	uint8_t lut_parity[256] = {
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
	};
};