//
// Title:	        Main emulator class
// Description:		
// Author:	        Dean Belfield
// Created:	        28/07/2026
// Last Updated:	28/07/2026
//
// Modinfo:

#pragma once

#include <cstdint>
#include <SDL2/SDL.h>

#include "defines.h"
#include "logger.h"
#include "ula.h"
#include "keyboard.h"
#include "memory.h"
#include "ports.h"
#include "tape.h"
#include "z80.h"

#define code "roms/48.rom"

using namespace std;

class Emulator {
public:
	Emulator(int scale, string filename);
	~Emulator();

	void	handleEvents(SDL_Event &e);
	void	run();
	bool    open(string filename);	
	
private:
	Logger*      logger;
	Keyboard*    keyboard;
	Ula*         ula;
	Z80*         z80;
	Mem*         mem;
	Ports*       ports;
	Tape*        tape;

	bool         step;
	bool         interrupts;	
};
