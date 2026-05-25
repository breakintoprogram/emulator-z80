#include "video.h"

video::video(uint8_t* ram) : ram(ram)
{
	int width = 256 * videoScale;
	int height = 192 * videoScale;

    SDL_Init(SDL_INIT_VIDEO);
	win = SDL_CreateWindow("emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(win, -1, 0);
	setColour(7);
	SDL_RenderClear(renderer);
}

video::~video() {
	SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();		
}

void video::render(bool flash) {
	for(int y = 0; y <= 191; y++) {
		for(int x = 0; x <= 31; x++) {
			uint16_t p = ((y & 0xC0) << 5) | ((y & 0x07) << 8) | ((y & 0x38) << 2) | x;
			uint16_t a = ((y & 0xF8) << 2) | x | 0x1800;
			uint8_t  c = ram[a];
			if(c < 0x80 || !flash) {
				renderByte(x<<3, y, c & 0x07, (c & 0x38) >> 3, ram[p]);
			}
			else {
				renderByte(x<<3, y, (c & 0x38) >> 3,  c & 0x07,ram[p]);
			}
		}
	}
	SDL_RenderPresent(renderer);
}

void video::setColour(uint8_t colour) {
	SDL_SetRenderDrawColor(renderer, palette[colour][0], palette[colour][1], palette[colour][2], 0x00);
}

void video::renderPoint(int x, int y, uint8_t colour) {
	setColour(colour);
	SDL_Rect p = {
		x * videoScale, y * videoScale, videoScale, videoScale
	};
	SDL_RenderFillRect(renderer, &p);
}

void video::renderByte(int x, int y, uint8_t inkColour, uint8_t paperColour, uint8_t byte) {
	for(int i = 0; i <= 7; i++) {
		renderPoint(x++, y, ((byte & 0x80) == 0x80) ? inkColour : paperColour);
		byte <<= 1;
	}
}

