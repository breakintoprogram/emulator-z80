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

void Logger::clear() {
	opcode.str("");
	opcode << hex << setfill('0');
	data.clear();
}

void Logger::setPC(uint16_t value) {
	pc = value;
}

void Logger::addData(uint8_t value) {
	data.emplace_back(value);
}

void Logger::setT(uint32_t value) {
	t = value;
}

void Logger::output() {
	stream << "PC: " << hex << setfill('0') << setw(4) << pc << " >";
	for (uint8_t b : data) {
		stream << " " << setw(2) << (uint16_t)b;
	}
	for (int i = data.size(); i < 4; i++) {
		stream << " ..";
	}
	stream << " " << setfill(' ') << setw(18) << left << opcode.str() << " = T" << dec << t << endl;
}

stringstream& Logger::getOpcode() {
	return opcode;
}

ostream& Logger::getStream() {
	return stream;
}