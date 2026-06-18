//
// Title:	        Spectrum 48K Ports
// Description:		Provides read/write functionality for Z80 ports
// Author:	        Dean Belfield
// Created:	        02/06/2026
// Last Updated:	02/06/2026
//
// Modinfo:

#pragma once

#include <cstdint>

#include "defines.h"

using namespace std;

class Ports {
public:
	Ports();

	uint8_t  in(uint16_t address);
	void     out(uint16_t address, uint8_t data);

	void     setFloating(uint8_t data);
	uint8_t* getPortsIn();
	uint8_t* getPortsOut();

private:
	uint8_t ports_in[256];	// Ports for the IN instruction, written to by the peripherals
	uint8_t ports_out[1];	// Ports for the OUT instruction, written to by the Z80
	uint8_t floating;		// Value for floating ports
};
