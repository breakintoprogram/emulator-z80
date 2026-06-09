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

#define TSPEED(s) (s/4.5)

using namespace std;

// Base tape segment class
//
class TapeSegment{
public:
    TapeSegment(uint8_t* ulaPort);
    virtual void play() = 0;
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
    ToneSegment(uint8_t* ulaPort, uint16_t pulseWidth, uint16_t pulseLength);
    void play() override;
protected:
private:
    uint16_t pulseWidth;
    uint16_t pulseLength;
    uint16_t count;
    bool     bit;
};

// Inherited pulse segment class
//
class PulseSegment : public TapeSegment {
public:
    PulseSegment(uint8_t* ulaPort, uint16_t pulseWidth0, uint16_t pulseWidth1);
    void play() override;
protected:
private:
    uint16_t pulseWidth0;
    uint16_t pulseWidth1;
};

// Inherited delay segment class
//
class DelaySegment : public TapeSegment {
public:
    DelaySegment(uint8_t* ulaPort, uint16_t delay);
    void play() override;
protected:
private:
    uint16_t delay;
};

// Inherited data segment class
//
class DataSegment : public TapeSegment {
public:
    DataSegment(uint8_t* ulaPort, ifstream& file, uint16_t blockSize, uint16_t pulseWidth0, uint16_t pulseWidth1);
    void play() override;
protected:
private:
    ifstream&       file;
    vector<uint8_t> data;
    uint16_t        pulseWidth0;
    uint16_t        pulseWidth1;
    uint8_t         bits;
    uint16_t        bitCount;
    uint16_t        pulseCount;
    bool            bit;
};

// The main tape class
//
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


