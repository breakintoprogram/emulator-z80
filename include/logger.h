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

#include "defines.h"

using namespace std;

class Logger {
public:
	Logger(ostream& stream);

	ostream& getStream();
private:
	ostream&  stream;		// The traceStream (defaults to cout)
};
