//
// Title:	        ZX Spectrum 48K emulator
// Description:		ZX Spectrum 48K emulator
// Author:	        Dean Belfield
// Created:	        22/05/2026
// Last Updated:	28/05/2026
//
// NB:
// Interactive ZX Spectrum ROM disassembly: https://skoolkid.github.io/rom/index.html
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
#include "z80.h"

#define code "roms/48.rom"
#define test "tests/z80doc.bin"

Keyboard* keyboard;
Ula*      ula;
Z80*      z80;
Mem*      mem;
Ports*    ports;

void cleanup() {
	delete mem;
	delete ports;
	delete keyboard;
	delete ula;
	delete z80;
}

int main()
{
	bool       quit = false;
	bool       step = false;
	bool       interrupts = true;
	int        turbo = 1;
	SDL_Event  e;

	mem = new Mem();

	if (!mem->load(0x0000, code)) {
		cout << "Error loading '" << code << "'." << endl;
		cleanup();
		return 1;
	}; 
	
	try  {
		ports = new Ports();
		ula = new Ula(mem, ports, 1);
		keyboard = new Keyboard(ports);
		z80 = new Z80(mem, ports);
	}
	catch(const exception& e) {
		cout << "Error: " << e.what() << endl;
		cleanup();
		return 1;
	}

	z80->reset();

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
							case SDLK_l: mem->load(0x8000, test); break;
							case SDLK_o: z80->dump(cout, true); break;
							case SDLK_r: z80->reset(); break;
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
				z80->run();
			}
			step = false;
		}
		//
		// Update the screen 
		//
		ula->render();
		if(ula->getvBlank()) {
			ula->setvBlank(false);
			if(!z80->getSingleStep() || interrupts) {
				z80->interruptRequest();
			}
		}
	}
	cleanup();	
	return 0;
}
