//
// Title:	        Spectrum 48K ULA
// Description:		Spectrum 48K ULA emulation
// Author:	        Dean Belfield
// Created:	        25/05/2026
// Last Updated:	28/05/2026
//
// Modinfo:

#include "ula.h"

#define HRES 256
#define VRES 192
#define CRES  32
#define HBORDER 48
#define VBORDER 56

Ula::Ula(Mem* mem, Ports* ports, int scale) :
	ram(mem->getRam() + 0x4000),
	ulaPort(ports->getPortsOut()),
	state(0),
	scanX(0),
	scanY(0),
	videoScale(scale),
	width(HRES + (HBORDER * 2)),
	height(VRES + (VBORDER * 2)),
	vBlank(false),
	frame(0)
{
    SDL_Init(SDL_INIT_VIDEO);
	win = SDL_CreateWindow(
		"emulator",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width * videoScale,
		height * videoScale,
		SDL_WINDOW_SHOWN
	);
	renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, width, height);
	SDL_RenderClear(renderer);
	if (SDL_LockTexture(texture, NULL, &pixels, &pitch) != 0) {
		throw runtime_error("Unable to lock texture");
	}
	pitch >>= 2;
}

Ula::~Ula() {
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();		
}

bool Ula::getvBlank() {
	return vBlank;
}

void Ula::setvBlank(bool b) {
	vBlank = b;
}

void Ula::render() {
	uint32_t borderColour = palette[(*ulaPort) & 0x07];
	bool    flash = frame & 0x10;

	switch(state) {
		//
		// Top border
		//
		case 0: { 
			renderByte(scanX, scanY, borderColour);
			scanX+=8;
			if(scanX == width) {
				scanX = 0;
				scanY++;
				if(scanY == VBORDER) {
					state = 1;		
				}
			}
		} break;
		//
		// Left border
		//
		case 1: {
			renderByte(scanX, scanY, borderColour);
			scanX+=8;
			if(scanX == HBORDER) {
				state = 2;
			}
		} break;
		//
		// Scanline
		//
		case 2: {
			uint16_t x = scanX - HBORDER;
			uint16_t y = scanY - VBORDER;
			uint16_t p = ((y & 0xC0) << 5) | ((y & 0x07) << 8) | ((y & 0x38) << 2) | (x >> 3);
			uint16_t a = ((y & 0xF8) << 2) | (x >> 3) | 0x1800;
			uint8_t  c = ram[a];
			uint8_t  ink = (c & 0x07) | ((c & 0x40) >> 3);
			uint8_t  paper = ((c & 0x78) >> 3);
			if(c < 0x80 || !flash) {
				renderByte(HBORDER + x, VBORDER + y, ink, paper, ram[p]);
			}
			else {
				renderByte(HBORDER + x, VBORDER + y, paper, ink ,ram[p]);
			}
			scanX+=8;
			if(scanX == HBORDER + HRES) {
				state = 3;
			}
		} break;
		//
		// Right border
		//
		case 3: {
			renderByte(scanX, scanY, borderColour);
			scanX+=8;
			if(scanX == width) {
				scanX = 0;
				scanY++;
				if(scanY < (VBORDER + VRES)) {
					state = 1;
				}
				else {
					state = 4;
				}
			}
		} break;
		//
		// Bottom border
		//
		case 4: {
			renderByte(scanX, scanY, borderColour);
			scanX+=8;
			if(scanX == width) {
				scanX = 0;
				scanY++;
				if(scanY == height) {
					scanY = 0;
					state = 0;
					SDL_UnlockTexture(texture);
					SDL_RenderCopy(renderer, texture, NULL, NULL);
					SDL_RenderPresent(renderer);
					if (SDL_LockTexture(texture, NULL, &pixels, &pitch) != 0) {
						throw runtime_error("Unable to lock texture");
					}
					pitch >>= 2;
					vBlank = true;
					frame++;
				}
			}
		} break;
	}
}

void Ula::renderByte(int x, int y, uint32_t borderColour) {
	uint32_t  index = x + (y * pitch);
	uint32_t* mappedPixels = (uint32_t*)pixels + index;

	*mappedPixels++ = borderColour;
	*mappedPixels++ = borderColour;
	*mappedPixels++ = borderColour;
	*mappedPixels++ = borderColour;
	*mappedPixels++ = borderColour;
	*mappedPixels++ = borderColour;
	*mappedPixels++ = borderColour;
	*mappedPixels   = borderColour;
}

void Ula::renderByte(int x, int y, uint8_t inkColour, uint8_t paperColour, uint8_t byte) {
	uint32_t  index = x + (y * pitch);	
	uint32_t* mappedPixels = (uint32_t*)pixels + index;

	uint32_t colours[2] = {
		palette[paperColour],
		palette[inkColour]
	};
	*mappedPixels++ = colours[(bool)(byte & 0x80)];
	*mappedPixels++ = colours[(bool)(byte & 0x40)];
	*mappedPixels++ = colours[(bool)(byte & 0x20)];
	*mappedPixels++ = colours[(bool)(byte & 0x10)];
	*mappedPixels++ = colours[(bool)(byte & 0x08)];
	*mappedPixels++ = colours[(bool)(byte & 0x04)];
	*mappedPixels++ = colours[(bool)(byte & 0x02)];
	*mappedPixels   = colours[(bool)(byte & 0x01)];
}

