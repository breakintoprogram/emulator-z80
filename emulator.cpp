//
// Title:	        Main emulator class
// Description:		
// Author:	        Dean Belfield
// Created:	        28/07/2026
// Last Updated:	28/07/2026
//
// Modinfo:

#include "emulator.h"

Emulator::Emulator(int scale, string filename) : step(false), interrupts(true) 
{
	logger = new Logger(cout);				// Logging class

	mem = new Mem(							// Memory interface with lambda for 'is RAM' check
		[](uint16_t address)->bool {
			return address >= 0x4000;
		}
	);
	ports = new Ports();					// I/O ports (keyboard, ULA)
	tape = new Tape(ports);					// Tape interface	

	// Load the ROM in
	//
	if (!mem->load(0x0000, code)) {
		throw runtime_error("Could not open ROM file");
	}; 	

	// Initialise the rest of the system
	//
	try  {
		ula = new Ula(mem, ports, scale);	// ULA (video)
		keyboard = new Keyboard(ports);		// Keyboard interface
		z80 = new Z80(mem, ports, logger);	// The Z80 CPU itself
	}
	catch(const runtime_error& e) {
		throw e;
	}

	// Setup the tape if a filename as been passed
	//
	if(filename.length() > 0) {
		if(!tape->open(filename)) {
			throw runtime_error("Could not open tape file");
		}
	}
	z80->reset();							// Reset the Z80	
}

Emulator::~Emulator()
{
	delete logger;
	delete mem;
	delete ports;
	delete keyboard;
	delete ula;
	delete tape;
	delete z80;	
}

void Emulator::handleEvents(SDL_Event &e) {
	switch (e.type) {
		//
		// Keypress down events
		//
		case SDL_KEYDOWN: {
			switch (e.key.keysym.sym) {
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
		//
		// Keypress up events
		//
		case SDL_KEYUP: {
			keyboard->press(e.key.keysym.sym, false);
		} break;
	}
}

void Emulator::run() {
	//
	// Process CPU cycle(s)
	//
	while(!ula->getvBlank()) {							// Loop until we get a vblank
		uint32_t tcnt = 0;
		uint16_t t;
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
	}
	//
	// Service interrupts
	//
	ula->setvBlank(false);								// Clear the vblank
	if (!z80->getSingleStep() || interrupts) {			// Provided we're not single-stepping and they're enabled
		z80->interruptRequest();
	}
}