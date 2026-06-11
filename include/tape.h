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
    TapeSegment(uint8_t* ulaPort);
    virtual void play(uint16_t tStates) = 0;
    bool isFinished();
protected:
    uint8_t* ulaPort;
    bool     finished;
    void     writeBit(uint8_t bit);
private:
};

// Inherited lead-in tone segment class
//
class ToneSegment : public TapeSegment {
public:
    ToneSegment(uint8_t* ulaPort, int16_t pulseWidth, int16_t pulseLength);
    void play(uint16_t tStates) override;
protected:
private:
    int16_t pulseWidth;
    int16_t pulseLength;
    int16_t  count;
    bool     bit;
};

// Inherited pulse segment class
//
class PulseSegment : public TapeSegment {
public:
    PulseSegment(uint8_t* ulaPort, int16_t pulseWidth0, int16_t pulseWidth1);
    void play(uint16_t tStates) override;
protected:
private:
    int16_t pulseWidth0;
    int16_t pulseWidth1;
};

// Inherited delay segment class
//
class DelaySegment : public TapeSegment {
public:
    DelaySegment(uint8_t* ulaPort, int16_t delay);
    void play(uint16_t tStates) override;
protected:
private:
    uint16_t delay;
};

// Inherited data segment class
//
class DataSegment : public TapeSegment {
public:
    DataSegment(uint8_t* ulaPort, ifstream& file, uint16_t blockSize, int16_t pulseWidth0, int16_t pulseWidth1);
    void play(uint16_t tStates) override;
protected:
private:
    ifstream&       file;
    vector<uint8_t> data;
    int16_t         pulseWidth0;
    int16_t         pulseWidth1;
    int16_t         pulseCount;
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
};


