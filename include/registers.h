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

	void A_add(uint8_t d);
	void A_adc(uint8_t d);
	void A_sub(uint8_t d);
	void A_sbc(uint8_t d);
	void A_and(uint8_t d);
	void A_xor(uint8_t d);
	void A_or (uint8_t d);
	void A_cp (uint8_t d);
	void A_neg();
	void A_not();
	void A_daa();

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