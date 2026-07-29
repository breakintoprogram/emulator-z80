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

#include "emulator.h"

using namespace std::chrono;
using namespace std::literals::chrono_literals;

// Global variables
//
static bool quit = false;
static auto speed = 20000us;

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
					case SDLK_F1: speed = 20000us; break;
					case SDLK_F2: speed = 10000us; break;
					case SDLK_F3: speed =  5000us; break;
					case SDLK_F4: speed =  2500us; break;					
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
	int       scale; 

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
	while (!quit) {
		auto nextTick = steady_clock::now() + speed;	
		loop((void *)emulator);	
		this_thread::sleep_until(nextTick);			// Wait until the next 1/50th of a second
		nextTick = steady_clock::now() + speed;		// Set the next tick to be from now		
	}
	return 0;
}
