#pragma once

#include <cstdint>
#include <SDL2/SDL.h>

#include "defines.h"

using namespace std;

class video {
public:
    video(uint8_t* ram, uint8_t* port);
    ~video();

    void render(bool flash);
private:
    SDL_Window* win = NULL;
    SDL_Renderer* renderer = NULL;
    
    uint8_t* ram = NULL;
    uint8_t* ulaPort = NULL;

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

    void setColour(uint8_t colour);
    void renderPoint(int x, int y, uint8_t colour);
    void renderByte(int x, int y, uint8_t inkColour, uint8_t paperColour, uint8_t byte);
    void renderBorderH(int x, int y, uint8_t colour);
    void renderBorderV(int y, uint8_t colour);
};