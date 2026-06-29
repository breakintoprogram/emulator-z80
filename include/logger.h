//
// Title:	        Spectrum 48K Debug
// Description:		Provides debugging and logging functionality
// Author:	        Dean Belfield
// Created:	        25/06/2026
// Last Updated:	25/06/2026
//
// Modinfo:

#pragma once

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

#include "defines.h"

using namespace std;

class Logger {
public:
	Logger(ostream& stream);

	ostream&      getStream();
	stringstream& getOpcode();
	void          clear(void);
	void          setPC(uint16_t value);
	void          addData(uint8_t value);
	void          setT(uint32_t value);
	void          output();

private:
	ostream&        stream;		// The traceStream (defaults to cout)
	uint16_t        pc;			// For the dump
	stringstream    opcode;
	vector<uint8_t> data;
	uint32_t        t;

};
