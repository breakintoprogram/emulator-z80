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
#include <vector>
#include <fstream> 
#include <filesystem>

#include "ports.h"
#include "defines.h"

using namespace std;

class TapeSegment{
public:
    TapeSegment(uint8_t* ulaPort);
    virtual void play() = 0;
    bool isFinished();
protected:
    uint8_t* ulaPort = NULL;
    bool     finished;
    void     writeBit(uint8_t bit);
private:
};

class ToneSegment : public TapeSegment {
public:
    ToneSegment(uint8_t* ulaPort, uint16_t pulseWidth, uint16_t pulseLength);
    void play() override;
protected:
private:
    uint16_t pulseWidth;
    uint16_t pulseLength;
    uint16_t count;
    bool     bit;
};

class Tape {
public:
    Tape(Ports* ports);

    bool open(string filename);
    bool openTAP(ifstream& file, uintmax_t filesize);
    void play();
private:
    uint8_t*  ulaPort = NULL;

    vector<unique_ptr<TapeSegment>> tape;
};


