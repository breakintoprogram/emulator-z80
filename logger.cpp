//
// Title:	        Spectrum 48K Debug
// Description:		Provides debugging and logging functionality
// Author:	        Dean Belfield
// Created:	        25/06/2026
// Last Updated:	25/06/2026
//
// Modinfo:

#include "logger.h"

Logger::Logger(ostream& stream) : stream(stream)
{
}

ostream& Logger::getStream() {
	return stream;
}