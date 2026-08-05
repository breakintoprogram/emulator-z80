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
	ula = new Ula(mem, ports, scale);	// ULA (video)
	keyboard = new Keyboard(ports);		// Keyboard interface
	z80 = new Z80(mem, ports, logger);	// The Z80 CPU itself

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
	uint16_t t;
	//
	// Process CPU cycle(s)
	//
	if (z80->getSingleStep()) {					// If in single-step mode
		if (step) {								// If a step has been requested by the user
			step = false;						// Flag the step as being done
			z80->run();							// Run the CPU
			t = z80->getT();
			tape->play(t);						// Play any inserted cassette
			ula->render(t);						// Render a number of pixels
		}
	}
	else {										// Otherwise if we're in free-running mode
		do {
			z80->run();							// Run the CPU
			t = z80->getT();					// Get the T-states
			tape->play(t);						// Play any inserted cassette
			ula->render(t);						// Render a number of pixels
		} while (!ula->getvBlank());			// Loop until we've got a vblank
	}
	//
	// Service interrupts
	//
	if (ula->getvBlank()) {						// If we've hit a vblank then
		ula->setvBlank(false);					// Clear the vblank
		//
		// Only service the interrupt if we're not single-stepping or interrupts are enabled in single-step mode
		//
		if (!z80->getSingleStep() || interrupts) {
			z80->interruptRequest();
		}		
	}
}

// Open a tape file
//
bool Emulator::open(string filename) {
	tape->stop();
	tape->close();
	if (!tape->open(filename)) {
		return false;
	}
	tape->start();
	return true;
}