//
// Title:	        ZX Spectrum 48K emulator
// Description:		ZX Spectrum 48K emulator
// Author:	        Dean Belfield
// Created:	        22/05/2026
// Last Updated:	28/05/2026
//
// Modinfo:

#include <iostream>
#include <memory>
#include <vector>
#include <fstream> 
#include <filesystem>
#include <chrono>
#include <thread>

#include "defines.h"
#include "logger.h"
#include "ula.h"
#include "keyboard.h"
#include "memory.h"
#include "ports.h"
#include "tape.h"
#include "z80.h"

#define code "roms/48.rom"

using namespace std::chrono;
using namespace std::literals::chrono_literals;

Logger*   logger;
Keyboard* keyboard;
Ula*      ula;
Z80*      z80;
Mem*      mem;
Ports*    ports;
Tape*     tape;

void cleanup() {
	delete logger;
	delete mem;
	delete ports;
	delete keyboard;
	delete ula;
	delete tape;
	delete z80;
}

int main(int argc, char* argv[])
{
	bool       quit = false;
	bool       step = false;
	bool       interrupts = true;
	auto       speed = 20000us;
	int        scale = 1;
	SDL_Event  e;
	auto       nextTick = steady_clock::now() + speed;

	logger = new Logger(cout);			// Logging class

	mem = new Mem(						// Memory interface with lambda for 'is RAM' check
		[](uint16_t address)->bool {
			return address >= 0x4000;
		}
	);
	ports = new Ports();				// I/O ports (keyboard, ULA)
	tape = new Tape(ports);				// Tape interface

	// Handle any command-line parameters
	//
	vector<string> arguments(argv + 1, argv + argc);

	for(string argument : arguments) {
		size_t delimiter = argument.find("=");
		string token(argument.substr(0, delimiter));
		string parameter(delimiter == string::npos ? "" : argument.substr(delimiter + 1));
	
		if (token == "s" || token =="scale") {
			scale = stoi(parameter);
			continue;
		}

		if (token == "l" || token == "load") {
			cout << "Loading tape file " << parameter << endl;
			if (!tape->open(parameter)) {
				cout << "Error: Could not open tape file" << endl;
				cleanup();
				return 1;
			}
			continue;
		}
	}

	// Load the ROM in
	//
	if (!mem->load(0x0000, code)) {
		cout << "Error loading '" << code << "'." << endl;
		cleanup();
		return 1;
	}; 
	
	// Initialise the rest of the system
	//
	try  {
		ula = new Ula(mem, ports, scale);	// ULA (video)
		keyboard = new Keyboard(ports);		// Keyboard interface
		z80 = new Z80(mem, ports, logger);	// The Z80 CPU itself
	}
	catch(const exception& e) {
		cout << "Error: " << e.what() << endl;
		cleanup();
		return 1;
	}

	z80->reset();						// Reset the Z80

	// The main loop
	//
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
					switch (e.key.keysym.sym) {
						case SDLK_F1: speed = 20000us; break;
						case SDLK_F2: speed = 10000us; break;
						case SDLK_F3: speed =  5000us; break;
						case SDLK_F4: speed =  2500us; break;
						case SDLK_F10: tape->start(); break;
						case SDLK_F11: tape->stop(); break;
						case SDLK_F12: z80->setSingleStep(true); break;
					}
					if(z80->getSingleStep()) {
						switch (e.key.keysym.sym) {
							case SDLK_RETURN: step = true; break;
							case SDLK_d: interrupts = false; break;
							case SDLK_e: interrupts = true; break;
							case SDLK_t: z80->setTrace(!z80->getTrace()); break;
							case SDLK_g: z80->setSingleStep(false); break;
							case SDLK_o: z80->dump(); break;
							case SDLK_r: z80->reset(); break;
						}
					}
					else {
						keyboard->press(e.key.keysym.sym, true);
					}
				} break;
				case SDL_KEYUP: {
					keyboard->press(e.key.keysym.sym, false);
				} break;
			}
		}
		//
		// Process CPU cycle(s)
		//
		uint32_t tcnt = 0;
		uint16_t t;
		try {
			if (z80->getSingleStep()) {						// Single-step mode
				if (step) {
					step = false;							// Flag the step as being done
					z80->run();								// Run the CPU
					t = z80->getT();
					tape->play(t);							// Play any inserted cassette
					ula->render(t);							// Render a number of pixels
				}
			}
			else {											// Normal running mode
				while (tcnt < 448 && !ula->getvBlank()) {	// Loop for around 2 scanlines (224 x 2)
					z80->run();								// Run the CPU
					t = z80->getT();						// Get the T-states
					tcnt += t;								// Add to the running total
					tape->play(t);							// Play any inserted cassette
					ula->render(t);							// Render a number of pixels
				}
			}
			if (ula->getvBlank()) {							// On the vblank
				ula->setvBlank(false);						// Service any interrupts
				if (!z80->getSingleStep() || interrupts) {	// Provided we're not single-stepping and they're enabled
					z80->interruptRequest();
				}
				this_thread::sleep_until(nextTick);			// Wait until the next 1/50th of a second
				nextTick = steady_clock::now() + speed;		// Set the next tick to be from now
			}
		}
		catch(const exception& e) {
			cout << "Error: " << e.what() << endl;
		}
	}
	cleanup();	
	return 0;
}
