#pragma once

#include <cstdint>
#include <SDL2/SDL.h>

#include "defines.h"

using namespace std;

class Ula {
public:
    Ula(uint8_t* ram, uint8_t* port);
    ~Ula();

    void render();
    bool getvBlank();
    void setvBlank(bool b);
private:
    SDL_Window* win = NULL;
    SDL_Renderer* renderer = NULL;
    
    uint8_t* ram = NULL;
    uint8_t* ulaPort = NULL;

    int  videoScale;
    int  state;
    int  scanX;
    int  scanY;
    int  width;
    int  height;
    bool vBlank;
    bool flash;

    uint8_t palette[16][3] = {
        //
        // The normal colours
        //
        { 0x00, 0x00, 0x00}, // Black
        { 0x00, 0x00, 0xC0}, // Blue
        { 0xC0, 0x00, 0x00}, // Red
        { 0xC0, 0x00, 0xC0}, // Magenta
        { 0x00, 0xC0, 0x00}, // Green
        { 0x00, 0xC0, 0xC0}, // Cyan
        { 0xC0, 0xC0, 0x00}, // Yellow
        { 0xC0, 0xC0, 0xC0}, // White
        //
        // The bright colours
        //
        { 0x00, 0x00, 0x00}, // Black
        { 0x00, 0x00, 0xFF}, // Blue
        { 0xFF, 0x00, 0x00}, // Red
        { 0xFF, 0x00, 0xFF}, // Magenta
        { 0x00, 0xFF, 0x00}, // Green
        { 0x00, 0xFF, 0xFF}, // Cyan
        { 0xFF, 0xFF, 0x00}, // Yellow
        { 0xFF, 0xFF, 0xFF}, // White
    };

    void setColour(uint8_t colour);
    void renderPoint(int x, int y, uint8_t colour);
    void renderByte(int x, int y, uint8_t borderColour);
    void renderByte(int x, int y, uint8_t inkColour, uint8_t paperColour, uint8_t byte);
};
