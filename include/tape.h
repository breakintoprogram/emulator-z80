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

// Base tape segment class
//
class TapeSegment{
public:
    TapeSegment(uint8_t* ulaPort, int16_t count);
    virtual bool play(uint16_t tStates);
protected:
    uint8_t* ulaPort;
	int16_t  count;
    void     writeBit(uint8_t bit);
private:
};

// Inherited pulse segment class
//
class PulseSegment : public TapeSegment {
public:
    PulseSegment(uint8_t* ulaPort, int16_t pulseWidth0, int16_t pulseWidth1, int16_t pulseCount);
    bool play(uint16_t tStates) override;
protected:
private:
    int16_t pulseWidth0;
    int16_t pulseWidth1;
	int16_t pulseCount;
	bool    bit;
};

// Inherited data segment class
//
class DataSegment : public TapeSegment {
public:
    DataSegment(uint8_t* ulaPort, ifstream& file, uint16_t blockSize, int16_t pulseWidth0, int16_t pulseWidth1);
    bool play(uint16_t tStates) override;
protected:
private:
    ifstream&       file;
    vector<uint8_t> data;
    int16_t         pulseWidth0;
    int16_t         pulseWidth1;
    uint8_t         bits;
    uint8_t         bitMask;
    bool            bit;
};

// The main tape class
//
class Tape {
public:
    Tape(Ports* ports);

    bool open(string filename);
    void play(uint16_t tStates);
private:
    uint8_t*  ulaPort = NULL;
    vector<unique_ptr<TapeSegment>> tape;

    bool openTAP(ifstream& file, uintmax_t filesize);
    bool openTZX(ifstream& file, uintmax_t filesize);

    bool readTZXStandardDataBlock(ifstream& file);
    bool readTZXTurboDataBlock(ifstream& file);
    bool readTZXTextDescription(ifstream& file);
    bool readTZXGroupStart(ifstream& file);
    bool readTZXGroupEnd(ifstream& file);
    bool readTZXPureTone(ifstream& file);
    bool readTZXPause(ifstream& file);
};
