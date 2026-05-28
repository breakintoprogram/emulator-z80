#include "ula.h"

#define HRES 256
#define VRES 192
#define CRES  32
#define HBORDER 48
#define VBORDER 56

ula::ula(uint8_t* ram, uint8_t* port) :
	ram(ram),
	ulaPort(port),
	state(0),
	scanX(0),
	scanY(0),
	videoScale(2),
	width(HRES + (HBORDER * 2)),
	height(VRES + (VBORDER * 2)),
	vBlank(false),
	flash(false)
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
	renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	setColour(7);
	SDL_RenderClear(renderer);
}

ula::~ula() {
	SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();		
}

bool ula::getvBlank() {
	return vBlank;
}

void ula::setvBlank(bool b) {
	vBlank = b;
}

void ula::render() {
	uint8_t borderColour = (*ulaPort) & 0x07;
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
					SDL_RenderPresent(renderer);
					SDL_RenderClear(renderer);
					vBlank = true;
					flash = !flash;
				}
			}
		} break;
	}
}

void ula::setColour(uint8_t colour) {
	SDL_SetRenderDrawColor(renderer, palette[colour][0], palette[colour][1], palette[colour][2], 0x00);
}

void ula::renderPoint(int x, int y, uint8_t colour) {
	setColour(colour);
	SDL_Rect p = {
		x * videoScale, y * videoScale, videoScale, videoScale
	};
	SDL_RenderFillRect(renderer, &p);
}

void ula::renderByte(int x, int y, uint8_t borderColour) {
	for(int i = 0; i <= 7; i++) {
		renderPoint(x++, y, borderColour);
	}	
}

void ula::renderByte(int x, int y, uint8_t inkColour, uint8_t paperColour, uint8_t byte) {
	for(int i = 0; i <= 7; i++) {
		renderPoint(x++, y, ((byte & 0x80) == 0x80) ? inkColour : paperColour);
		byte <<= 1;
	}
}

