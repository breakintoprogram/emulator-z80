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
		delete mem;
		return 1;
	}; 

	ports = new Ports();
	keyboard = new Keyboard(ports);
	ula = new Ula(mem, ports, 1);
	z80 = new Z80(mem, ports);

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
							case SDLK_t: z80->setTrace(true); break;
							case SDLK_g: z80->setSingleStep(false); break;
							case SDLK_l: mem->load(0x8000, test); break;
							case SDLK_o: z80->dump(true); break;
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
		for(int i = 0; i < turbo; i++) {
			//
			// Process one cycle of the CPU
			//
			if(!z80->getSingleStep() || step) {
				//
				// Execute one instruction
				//
				do {
					z80->debug();
					z80->fetch();
					z80->decode();
					z80->execute();
				} while(z80->getCycle() > 0);
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
				z80->interruptRequest(0x38);
			}
		}
	}

	delete mem;
	delete ports;
	delete keyboard;
	delete ula;
	delete z80;
	
	return 0;
}
