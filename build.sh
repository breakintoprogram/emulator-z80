#!/bin/sh

g++ --std=c++23 -O3 -Iinclude -I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2 *.cpp -obin/emulator-z80