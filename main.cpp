//
// Title:	        ZX Spectrum 48K emulator
// Description:		ZX Spectrum 48K emulator
// Author:	        Dean Belfield
// Created:	        22/05/2026
// Last Updated:	28/05/2026
//
// Modinfo:

#include <iostream>
#include <memory>
#include <vector>
#include <fstream> 
#include <filesystem>

#include "defines.h"
#include "ula.h"
#include "keyboard.h"
#include "memory.h"
#include "ports.h"
#include "tape.h"
#include "z80.h"

#define code "roms/48.rom"

Keyboard* keyboard;
Ula*      ula;
Z80*      z80;
Mem*      mem;
Ports*    ports;
Tape*     tape;

void cleanup() {
	delete mem;
	delete ports;
	delete keyboard;
	delete ula;
	delete tape;
	delete z80;
}

int main(int argc, char* argv[])
{
	bool       quit = false;
	bool       step = false;
	bool       interrupts = true;
	int        turbo = 1;
	int        scale = 1;
	string     tapeFile;
	SDL_Event  e;

	mem = new Mem();

	// Handle any command-line parameters
	//
	vector<string> arguments(argv + 1, argv + argc);

	for(string argument : arguments) {
		size_t delimiter = argument.find("=");
		string token(argument.substr(0, delimiter));
		string parameter(delimiter == string::npos ? "" : argument.substr(delimiter + 1));
	
		if(token == "s" || token =="scale") {
			scale = stoi(parameter);
			continue;
		}

		if (token == "l" || token == "load") {
			tapeFile = parameter;
			continue;
		}
	}

	// Load the ROM in
	//
	if (!mem->load(0x0000, code)) {
		cout << "Error loading '" << code << "'." << endl;
		cleanup();
		return 1;
	}; 
	
	// Initialise the rest of the system
	//
	try  {
		ports = new Ports();				// I/O ports (keyboard, ULA)
		ula = new Ula(mem, ports, scale);	// ULA (video)
		keyboard = new Keyboard(ports);		// Keyboard interface
		tape = new Tape(ports);				// Tape interface
		z80 = new Z80(mem, ports);			// The Z80 CPU itself
	}
	catch(const exception& e) {
		cout << "Error: " << e.what() << endl;
		cleanup();
		return 1;
	}

	z80->reset();						// Reset the Z80

	// The main loop
	//
	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			switch (e.type) {
				//
				// User closes window
				//
				case SDL_QUIT: {
					quit = true;
				} break;
				//
				// Keyboard events
				//
				case SDL_KEYDOWN: {
					switch (e.key.keysym.sym) {
						case SDLK_F1: turbo = 1; break;
						case SDLK_F2: turbo = 2; break;
						case SDLK_F3: turbo = 4; break;
						case SDLK_F4: turbo = 8; break;
						case SDLK_F5: turbo = 16; break;
						case SDLK_F12: z80->setSingleStep(true); break;
					}
					if(z80->getSingleStep()) {
						switch (e.key.keysym.sym) {
							case SDLK_RETURN: step = true; break;
							case SDLK_d: interrupts = false; break;
							case SDLK_e: interrupts = true; break;
							case SDLK_t: z80->setTrace(!z80->getTrace()); break;
							case SDLK_g: z80->setSingleStep(false); break;
							case SDLK_o: z80->dump(cout, true); break;
							case SDLK_r: z80->reset(); break;
							case SDLK_p: tape->open(tapeFile); break;
						}
					}
					else {
						keyboard->press(e.key.keysym.sym, true);
					}
				} break;
				case SDL_KEYUP: {
					keyboard->press(e.key.keysym.sym, false);
				} break;
			}
		}
		//
		// Process CPU cycle(s)
		//
		for(int i = 0; i < turbo; i++) {
			if(!z80->getSingleStep() || step) {
				try {
					z80->run();										// Run the CPU
					tape->play(z80->getT());						// Play any inserted cassette
					ula->render(z80->getT());						// Render a number of pixels
					if(ula->getvBlank()) {							// On the vblank
						ula->setvBlank(false);						// Service any interrupts
						if(!z80->getSingleStep() || interrupts) {	// Provided we're not single-stepping and they're enabled
							z80->interruptRequest();
						}
					}
				}
				catch(const exception& e) {
					cout << "Error: " << e.what() << endl;
				}
			}
			step = false;
		}
	}
	cleanup();	
	return 0;
}
