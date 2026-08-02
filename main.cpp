//
// Title:	        ZX Spectrum 48K emulator
// Description:		ZX Spectrum 48K emulator
// Author:	        Dean Belfield
// Created:	        22/05/2026
// Last Updated:	02/08/2026
//
// Modinfo:
// 02/08/2026:		Added Emscripten support

#include <iostream>
#include <memory>
#include <vector>
#include <fstream> 
#include <filesystem>
#include <chrono>
#include <thread>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "emulator.h"

using namespace std::chrono;
using namespace std::literals::chrono_literals;

// Global variables
//
static bool quit = false;

// Function prototypes
//
void loop(void * arg);

// Set the emulation speed
//
#ifdef __EMSCRIPTEN__
void setSpeed(Emulator* emulator, int speed) {
	emscripten_cancel_main_loop();
	emscripten_set_main_loop_arg(loop, emulator, speed, 1);
}
#else
static auto speed = 20000us;
#endif 

// The loop
// Parameters:
// - arg: void pointer to an Emulator object
//
void loop(void* arg) {
	SDL_Event e;

	Emulator* emulator = (Emulator*)arg;

	while (SDL_PollEvent(&e) != 0) {
		switch (e.type) {
			case SDL_QUIT: {
				quit = true;
			} break;	
			case SDL_KEYDOWN: {
				switch (e.key.keysym.sym) {
					#ifdef __EMSCRIPTEN__
					case SDLK_F1: setSpeed(emulator, 50); break;
					case SDLK_F2: setSpeed(emulator, 100); break;
					case SDLK_F3: setSpeed(emulator, 200); break;
					case SDLK_F4: setSpeed(emulator, 400); break;
					#else 
					case SDLK_F1: speed = 20000us; break;
					case SDLK_F2: speed = 10000us; break;
					case SDLK_F3: speed =  5000us; break;
					case SDLK_F4: speed =  2500us; break;
					#endif
				}
			} break;
		}
		emulator->handleEvents(e);
	}
	try {
		emulator->run();
	}
	catch(const runtime_error& e) {
		cout << "Error: " << e.what() << endl;
	}
}

// The main function
// Parameters:
// - argc: argument count
// - argv: argument array
//
int main(int argc, char* argv[])
{
	Emulator* emulator;
	string    filename;
	int       scale = 1; 

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
			filename = parameter;
			continue;
		}
	}

	try {
		emulator = new Emulator(scale, filename);
	}
	catch(const runtime_error& e) {
		cout << "Error: " << e.what() << endl;
		delete emulator;
		return 0;
	}

	// The main loop
	//
	#ifdef __EMSCRIPTEN__
		emscripten_set_main_loop_arg(loop, emulator, 50, 1);
	#else 
	while (!quit) {
		auto nextTick = steady_clock::now() + speed;	
		loop((void *)emulator);	
		this_thread::sleep_until(nextTick);			// Wait until the next 1/50th of a second
		nextTick = steady_clock::now() + speed;		// Set the next tick to be from now		
	}
	#endif	

	return 0;
}
