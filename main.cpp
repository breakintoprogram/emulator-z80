// https://www.asm80.com/
// NB: Debug only works with the C/C++ extension version 1.29.3 - do not upgrade it!

#include <iostream>
#include <memory>
#include <vector>
#include <fstream> 
#include <filesystem>
#include <SDL2/SDL.h>

#include "defines.h"
#include "cpu.h"

// https://skoolkid.github.io/rom/index.html
//
#define code "roms/48.rom"

uint8_t ram[RAM_SIZE];

SDL_Window *win = NULL;
SDL_Renderer *renderer = NULL;

int videoScale = 2;

uint8_t palette[8][3] = {
	{ 0x00, 0x00, 0x00}, // Black
	{ 0x00, 0x00, 0xFF}, // Blue
	{ 0xFF, 0x00, 0x00}, // Red
	{ 0xFF, 0x00, 0xFF}, // Magenta
	{ 0x00, 0xFF, 0x00}, // Green
	{ 0x00, 0xFF, 0xFF}, // Cyan
	{ 0xFF, 0xFF, 0x00}, // Yellow
	{ 0xFF, 0xFF, 0xFF}, // White
};

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

void setColour(uint8_t colour) {
	SDL_SetRenderDrawColor(renderer, palette[colour][0], palette[colour][1], palette[colour][2], 0x00);
}

void renderPoint(int x, int y, uint8_t colour) {
	setColour(colour);
	SDL_Rect p = {
		x * videoScale, y * videoScale, videoScale, videoScale
	};
	SDL_RenderFillRect(renderer, &p);
}

void renderByte(int x, int y, uint8_t inkColour, uint8_t paperColour, uint8_t byte) {
	for(int i = 0; i <= 7; i++) {
		renderPoint(x++, y, ((byte & 0x80) == 0x80) ? inkColour : paperColour);
		byte <<= 1;
	}
}

void renderULA(bool flash) {
	for(int y = 0; y <= 191; y++) {
		for(int x = 0; x <= 31; x++) {
			uint16_t p = 0x4000 | ((y & 0xC0) << 5) | ((y & 0x07) << 8) | ((y & 0x38) << 2) | x;
			uint16_t a = 0x5800 | ((y & 0xF8) << 2) | x;
			uint8_t  c = ram[a];
			if(c < 0x80 || !flash) {
				renderByte(x<<3, y, c & 0x07, (c & 0x38) >> 3, ram[p]);
			}
			else {
				renderByte(x<<3, y, (c & 0x38) >> 3,  c & 0x07,ram[p]);
			}
		}
	}
}

bool initGraphics() {
	int width = 256 * videoScale;
	int height = 192 * videoScale;

    SDL_Init(SDL_INIT_VIDEO);
	win = SDL_CreateWindow("emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(win, -1, 0);
	setColour(7);
	SDL_RenderClear(renderer);
	return true;
}

void destroyGraphics() {
	SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();		
}

int main()
{
	if (!load(ram, code)) {
		cout << "Error loading '" << code << "'." << endl;
		return false;
	}; 
	cpu z80(ram);

	initGraphics();

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
						switch (e.key.keysym.sym) {
							case SDLK_SPACE: z80.setPort(0x7FFE, 0b00011110); break;
							case SDLK_RETURN: z80.setPort(0xBFFE, 0b00011110); break;
						}
					}
				} break;
				case SDL_KEYUP: {
					switch (e.key.keysym.sym) {
						default: z80.setPort(0x0000, 0xFF); break;
					}
				}
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
			renderULA(flash);
			SDL_RenderPresent(renderer);
			flash = !flash;
		}
		count= ++count % 16384;
	}

	destroyGraphics();
}
