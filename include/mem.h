//
// Title:	        Spectrum 48K RAM
// Description:		Provides read/write functionality for RAM
// Author:	        Dean Belfield
// Created:	        02/06/2026
// Last Updated:	02/06/2026
//
// Modinfo:

#pragma once

#include <cstdint>

#include "defines.h"

using namespace std;

class Mem {
public:
    uint8_t  readByte(uint16_t address);
    uint16_t readWord(uint16_t address);
    void     write(uint16_t address, uint8_t data);
    void     write(uint16_t address, uint16_t data);
    bool     load(uint16_t address, string filename);
    uint8_t* getRam();
private:
    uint8_t ram[RAM_SIZE];
};
