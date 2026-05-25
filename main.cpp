// https://www.asm80.com/
// NB: Debug only works with the C/C++ extension version 1.29.3 - do not upgrade it!

#include <iostream>
#include <memory>
#include <vector>
#include <fstream> 
#include <filesystem>

#include "defines.h"
#include "video.h"
#include "keyboard.h"
#include "cpu.h"

// https://skoolkid.github.io/rom/index.html
//
#define code "roms/48.rom"

uint8_t ram[RAM_SIZE];
uint8_t ports[256];

bool load(uint8_t* buffer, string filename) {
	if (!filesystem::exists(filename)) {
		return false;
	}
	auto filesize = filesystem::file_size(filename);
	if(filesize > RAM_SIZE) {
		return false;
	}
	ifstream file(filename, ios::binary);
	if (file.read((char *)&ram[0], filesize)) {
		return true;
	}
	return false;
}

int main()
{
	if (!load(ram, code)) {
		cout << "Error loading '" << code << "'." << endl;
		return false;
	}; 
	keyboard keyboard(ports);
	video video(ram + 0x4000);
	cpu z80(ram, ports);

	z80.reset();

	uint16_t   count = 0;
	bool       flash = false;
	bool       quit = false;
	bool       step = false;
	SDL_Event  e;

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
					if(z80.getSingleStep()) {
						switch (e.key.keysym.sym) {
							case SDLK_RETURN: step = true; break;
							case SDLK_b: z80.setSingleStep(true); break;
							case SDLK_g: z80.setSingleStep(false); break;
						}
					}
					else {
						keyboard.press(e.key.keysym.sym, true);
					}
				} break;
				case SDL_KEYUP: {
					keyboard.press(e.key.keysym.sym, false);
				} break;
			}
		}
		//
		// Process one cycle of the CPU
		//
		if(!z80.getSingleStep() || step) {
			//
			// Execute one instruction
			//
			do {
				z80.debug();
				z80.fetch();
				z80.decode();
				z80.execute();
			} while(z80.getCycle() > 0);
		}
		step = false;
		//
		// Update the screen every so often
		//
		if(count == 0) {
			z80.interruptRequest(0x38);
			video.render(flash);
			flash = !flash;
		}
		count= ++count % 16384;
	}
}
