//
// Title:	        Spectrum 48K Tape
// Description:		Provides basic tape fuctionality
// Author:	        Dean Belfield
// Created:	        08/06/2026
// Last Updated:	08/06/2026
//
// Modinfo:

#pragma once

#include <cstdint>
#include <iostream>
#include <fstream> 
#include <filesystem>

#include "ports.h"
#include "defines.h"

using namespace std;

class Tape {
public:
    Tape(Ports* ports);

    bool open(string filename);
    void close();
    void writeBit(uint8_t bit);
    void play();
private:
    uint8_t*  ulaPort = NULL;
    uintmax_t filesize;
    ifstream  fs;
    uint16_t  state;
};
