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

#define RAM_SIZE 0x10000

#define NOP uint16_t __nop = 0

union REG {
	uint16_t W;
	struct {
		uint8_t L;
		uint8_t H;
	};
	struct {
		uint8_t C : 1;	// 1 if carry, otherwise 0
		uint8_t N : 1;	// 1 if last operation was a subtract, otherwise 0
		uint8_t P : 1;	// 1 if the result has an even number of 1 bits set, or a signed operation overflows
		uint8_t F3 : 1;	// Copies bit 3 of the result
		uint8_t B : 1;	// 1 if a carry/borrow occurred between bits 3 and 4 (for BCD operations)
		uint8_t F5 : 1;	// Copies bit 5 of the result
		uint8_t Z : 1;	// 1 if the result is zero
		uint8_t S : 1;	// 1 if the result is negative
		uint8_t A;		// The accumulator
	};
	REG() { W = 0; };
};