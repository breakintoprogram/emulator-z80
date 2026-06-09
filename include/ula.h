//
// Title:	        Spectrum 48K ULA
// Description:		Spectrum 48K ULA emulation
// Author:	        Dean Belfield
// Created:	        25/05/2026
// Last Updated:	28/05/2026
//
// Modinfo

#pragma once

#include <cstdint>
#include <SDL2/SDL.h>

#include "mem.h"
#include "ports.h"
#include "defines.h"

using namespace std;

class Ula {
public:
    Ula(Mem* mem, Ports* ports, int scale);
    ~Ula();

    void render();
    bool getvBlank();
    void setvBlank(bool b);
private:
    SDL_Window* win = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Texture* texture = NULL;
    
    uint8_t* ram = NULL;
    uint8_t* ulaPort = NULL;

    int     videoScale;
    int     state;
    int     scanX;
    int     scanY;
    int     width;
    int     height;
    uint8_t frame;
    bool    vBlank;
    void*   pixels;
    int     pitch;

    uint32_t palette[16] = {
        //
        // The normal colours
        //
        0x000000, // Black
        0x0000C0, // Blue
        0xC00000, // Red
        0xC000C0, // Magenta
        0x00C000, // Green
        0x00C0C0, // Cyan
        0xC0C000, // Yellow
        0xC0C0C0, // White
        //
        // The bright colours
        //
        0x000000, // Black
        0x0000FF, // Blue
        0xFF0000, // Red
        0xFF00FF, // Magenta
        0x00FF00, // Green
        0x00FFFF, // Cyan
        0xFFFF00, // Yellow
        0xFFFFFF, // White
    };

    void renderByte(int x, int y, uint32_t borderColour);
    void renderByte(int x, int y, uint8_t inkColour, uint8_t paperColour, uint8_t byte);
};
