//
// Title:	        Spectrum 48K RAM
// Description:		Provides read/write functionality for RAM
// Author:	        Dean Belfield
// Created:	        02/06/2026
// Last Updated:	02/06/2026
//
// Modinfo:

#include "mem.h"

uint8_t Mem::readByte(uint16_t address) {
    return ram[address];
}

uint16_t Mem::readWord(uint16_t address) {
    return ram[address] | ram[address + 1] << 8;
}

void Mem::write(uint16_t address, uint8_t data) {
    if(address >= 0x4000) {
        ram[address] = data;
    }
}

void Mem::write(uint16_t address, uint16_t data) {
    uint8_t lsb = data & 0xFF;
    uint8_t msb = data >> 8;
    write(address, lsb);
    write(address + 1, msb);
}

bool Mem::load(uint16_t address, string filename) {
	if (!filesystem::exists(filename)) {
		return false;
	}
	auto filesize = filesystem::file_size(filename);
	if(filesize > RAM_SIZE) {
		return false;
	}
	ifstream file(filename, ios::binary);
	if (file.read((char *)(ram + address), filesize)) {
		return true;
	}
	return false;
}

uint8_t* Mem::getRam() {
    return ram;
}