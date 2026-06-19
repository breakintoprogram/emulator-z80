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
	ports(ports),
	state(0),
	scanX(0),
	scanY(0),
	scanP(nullptr),
	videoScale(scale),
	width(HRES + (HBORDER * 2)),
	height(VRES + (VBORDER * 2)),
	vBlank(false),
	frame(0),
	tcount(0)
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
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, width, height);
	SDL_RenderClear(renderer);
	resetState();
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

bool Ula::isReadingRAM() {
	return state == 2;
}

void Ula::renderByte(uint32_t border) {
	for(int i = 0; i < 8; i++) {
		*scanP++ = border;	
	}
}

void Ula::renderByte(uint32_t ink, uint32_t paper, bool flash, uint8_t pixelData) {
	uint32_t colours[2] = {
		palette[flash ? paper : ink],
		palette[flash ? ink : paper]
	};
	for(uint8_t m = 0x80; m != 0x00; m >>= 1) {
		*scanP++ = colours[!!(pixelData & m)];
	}
}

void Ula::resetState() {
	if (SDL_LockTexture(texture, NULL, &pixels, &pitch) != 0) {
		throw runtime_error("Unable to lock texture");
	}
	scanP = (uint32_t*)pixels;				// Set pointer to top left of screen
	scanC = width * VBORDER / 8;			// Top border size
	state = 0;								// Initial state is 0
}

void Ula::renderTexture() {
	SDL_UnlockTexture(texture);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

// Maintain a tally of T-states for the renderer and render the appropriate number of pixels
// The tally in tcount is multiplied by 1024 for limited fixed point precision
// Parameters: 
// - tStates: Number of T-states to deduct from the tally
//
void Ula::render(uint16_t tStates) {
	const uint16_t td = 224 * 1024 / 44;	// The number of T-states for 8 pixels
	tcount += (tStates * 1024);				// Add last T-states executed to the running tally, multiplied by 1024
	while(tcount >= td && ! getvBlank()) {	// Whilst we've got T-states to run, and there's not been a vblank then
		render();							// Render 8 pixels of the screen
		tcount -= td;						// Deduct 8 pixels worth of T-states from the tally
	}
}

// State machine to render the screen 8 pixels at a time, including the borders
//
void Ula::render() {
	uint8_t* borderPort = ports->getPortsOut();
	uint32_t borderColour = palette[(*borderPort) & 0x07];
	bool    flash = frame & 0x10;

	switch(state) {
		//
		// Top border
		//
		case 0: { 
			renderByte(borderColour);		// Render 8 pixels worth of border
			if(--scanC == 0) {				// Count down until we've filled the top border
				scanC = HBORDER / 8;		// Set up the next state
				scanY = 0;
				state = 1;
			}
		} break;
		//
		// Left border
		//
		case 1: {
			renderByte(borderColour);		// Render 8 pixels worth of border
			if(--scanC == 0) {				// Count down until we've filled the left border
				scanC = HRES / 8;			// Set up the next state
				scanX = 0;
				state = 2;
			}
		} break;
		//
		// Scanline
		//
		case 2: {
			// First get the location of the pixel and attribute data in the Spectrum memory map
			//
			uint16_t specPixels = ((scanY & 0xC0) << 5) | ((scanY & 0x07) << 8) | ((scanY & 0x38) << 2) | scanX;
			uint16_t specAttribute = ((scanY & 0xF8) << 2) | scanX | 0x1800;
			//
			// Fetch the attribute value and isolate the ink and paper values
			// This function also adds the bright value to both
			//
			uint8_t  attr = ram[specAttribute];
			uint8_t  ink = (attr & 0x07) | ((attr & 0x40) >> 3);
			uint8_t  paper = ((attr & 0x78) >> 3);
			//
			// Render a single byte's worth of data, taking into account the flash attribute bit
			//
			renderByte(ink, paper, attr < 0x80 || !flash, ram[specPixels]);
			scanX++;
			if(--scanC == 0) {				// Count down until we've filled the scanline
				scanC = HBORDER / 8;		// Set up the next state
				state = 3;
				attr = 0xFF;				// We've done, so reset the floating bus value
			}
			ports->setFloating(attr);		// Set the floating bus
		} break;
		//
		// Right border
		//
		case 3: {
			renderByte(borderColour);		// Render 8 pixels worth of border
			if(--scanC == 0) {				// Count down until we've filled the right border
				if(++scanY < VRES) {		// If we've not done 192 lines then
					scanC = HBORDER / 8;	// Set up the next state
					state = 1;				// For the left border
				}
				else {						// Otherwise, switch to the bottom border state
					scanC = width * VBORDER / 8;
					state = 4;
				}
			}
		} break;
		//
		// Bottom border
		//
		case 4: {
			renderByte(borderColour);		// Render the bottom border
			if(--scanC == 0) {				// Count down until we've filled it
				//
				// This bit unlocks the texture we've just drawn to, and
				// blits it to the screen, finally locking the texture so
				// we can draw the next frame
				//
				renderTexture();
				resetState();
				vBlank = true;				// Flag a vblank at this point
				frame++;					// Increment the frame counter
			}
		} break;
	}
}