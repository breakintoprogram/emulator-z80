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
#include "z80.h"

#define code "roms/48.rom"
#define test "tests/z80doc.bin"

Keyboard* keyboard;
Ula*      ula;
Z80*      z80;
uint8_t*  ram;
uint8_t*  ports;

bool load(uint8_t* buffer, string filename) {
	if (!filesystem::exists(filename)) {
		return false;
	}
	auto filesize = filesystem::file_size(filename);
	if(filesize > RAM_SIZE) {
		return false;
	}
	ifstream file(filename, ios::binary);
	if (file.read((char *)buffer, filesize)) {
		return true;
	}
	return false;
}

void decodeOut(uint16_t addr, uint8_t v) {
	ports[0] = v;
}

uint8_t decodeIn(uint16_t addr) {
	return ports[addr >> 8];
}

int main()
{
	bool       quit = false;
	bool       step = false;
	bool       interrupts = true;
	int        turbo = 1;
	SDL_Event  e;

	ram = new uint8_t[RAM_SIZE];

	if (!load(ram, code)) {
		cout << "Error loading '" << code << "'." << endl;
		delete[] ram;
		return 1;
	}; 

	ports = new uint8_t[256];
	keyboard = new Keyboard(ports);
	ula = new Ula(ram + 0x4000, ports, 1);
	z80 = new Z80(ram, &decodeOut, &decodeIn);

    for(int i=0; i<=255; i++) {
        ports[i] = 0xFF;
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
							case SDLK_t: z80->setTrace(true); break;
							case SDLK_g: z80->setSingleStep(false); break;
							case SDLK_l: load(ram + 0x8000, test); break;
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

	delete[] ram;
	delete[] ports;
	delete   keyboard;
	delete   ula;
	delete   z80;
	
	return 0;
}
