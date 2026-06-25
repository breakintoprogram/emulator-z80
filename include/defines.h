//
// Title:	        ZX Spectrum 48K defines
// Description:		Global defines
// Author:	        Dean Belfield
// Created:	        22/05/2026
// Last Updated:	28/05/2026
//
// Modinfo:

#pragma once

#include <cstdint>

#define EMULATOR_NAME "Speculation"
#define RAM_SIZE 0x10000
#define LOG(s) logger->getStream() << s
#define LOGIF(b, s) if (b) LOG(s)
