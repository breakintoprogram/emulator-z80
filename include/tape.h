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

// Inherited data segment class
//
class DataSegment : public TapeSegment {
public:
    DataSegment(uint8_t* ulaPort, ifstream& file, uintmax_t& bytesRemaining);
    void play() override;
protected:
private:
    ifstream&       file;
    vector<uint8_t> data;
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


