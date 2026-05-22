#pragma once

#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <algorithm>
#include <cstdint>

#include "defines.h"
#include "registers.h"

using namespace std;

class cpu {
public:
	cpu(uint8_t* ram);

	void addBreakpoint(uint16_t a);
	void interruptRequest(uint8_t i);

	void reset();
	void debug();
	void fetch();
	void decode();
	void execute();

private:
	uint8_t* ram;
	vector<uint16_t> breakpoints;

	// Look-up tables for the x and z decode stage
	//
	vector<vector<void (cpu::*)()>> lut_xz = {
		{ &cpu::execute_x0z0, &cpu::execute_x0z1, &cpu::execute_x0z2, &cpu::execute_x0z3, &cpu::execute_x0z4, &cpu::execute_x0z5, &cpu::execute_x0z6, &cpu::execute_x0z7 },
		{ &cpu::execute_x1__, &cpu::execute_x1__, &cpu::execute_x1__, &cpu::execute_x1__, &cpu::execute_x1__, &cpu::execute_x1__, &cpu::execute_x1__, &cpu::execute_x1__ },
		{ &cpu::execute_x2__, &cpu::execute_x2__, &cpu::execute_x2__, &cpu::execute_x2__, &cpu::execute_x2__, &cpu::execute_x2__, &cpu::execute_x2__, &cpu::execute_x2__ },
		{ &cpu::execute_x3z0, &cpu::execute_x3z1, &cpu::execute_x3z2, &cpu::execute_x3z3, &cpu::execute_x3z4, &cpu::execute_x3z5, &cpu::execute_x3z6, &cpu::execute_x3z7 }
	};

	// Look-up table for ALU operations
	//
	vector<void (registers::*)(uint8_t)> lut_alu = {
		&registers::A_add,
		&registers::A_adc,
		&registers::A_sub,
		&registers::A_sbc,
		&registers::A_and,
		&registers::A_xor,
		&registers::A_or,
		&registers::A_cp
	};

	// Look-up table for ROT operations
	//
	vector<void (cpu::*)(uint8_t *)> lut_rot = {
		&cpu::rlc,
		&cpu::rrc,
		&cpu::rl,
		&cpu::rr,
		&cpu::sla,
		&cpu::sra,
		&cpu::sll,
		&cpu::srl
	};

	// Look-up table for block operations
	//
	vector<vector<void (cpu::*)()>> lut_bli = {
		{ &cpu::ldi,  &cpu::cpi,  &cpu::ini,  &cpu::outi },
		{ &cpu::ldd,  &cpu::cpd,  &cpu::ind,  &cpu::outd },
		{ &cpu::ldir, &cpu::cpir, &cpu::inir, &cpu::otir },
		{ &cpu::lddr, &cpu::cpdr, &cpu::indr, &cpu::otdr }
	};

	// Look-up table for conditions
	//
	vector<bool (registers::*)()> lut_cc = {
		&registers::F_NZ,
		&registers::F_Z,
		&registers::F_NC,
		&registers::F_C,
		&registers::F_PO,
		&registers::F_PE,
		&registers::F_P,
		&registers::F_M,
	};

	// 8-bit register lookup
	//
	uint8_t* t_r[3][8] = {
		{ &reg.BC.H, &reg.BC.L, &reg.DE.H, &reg.DE.L, &reg.HL.H, &reg.HL.L, NULL, &reg.AF.H },	// NULL is (HL)
		{ &reg.BC.H, &reg.BC.L, &reg.DE.H, &reg.DE.L, &reg.IX.H, &reg.IX.L, NULL, &reg.AF.H },	// NULL is (IX)
		{ &reg.BC.H, &reg.BC.L, &reg.DE.H, &reg.DE.L, &reg.IY.H, &reg.IY.L, NULL, &reg.AF.H }	// NULL is (IY)
	};

	// 16-bit register lookup
	//
	uint16_t * t_rp[8][4] = {
		{ &reg.BC.W, &reg.DE.W, &reg.HL.W, &reg.SP   },
		{ &reg.BC.W, &reg.DE.W, &reg.IX.W, &reg.SP   },
		{ &reg.BC.W, &reg.DE.W, &reg.IY.W, &reg.SP   },
		{ &reg.BC.W, &reg.DE.W, &reg.HL.W, &reg.AF.W },
		{ &reg.BC.W, &reg.DE.W, &reg.IX.W, &reg.AF.W },
		{ &reg.BC.W, &reg.DE.W, &reg.IY.W, &reg.AF.W },
		{ &reg.BC.W, &reg.DE.W, &reg.HL.W, &reg.IX.W },
		{ &reg.BC.W, &reg.DE.W, &reg.HL.W, &reg.IY.W }
	};

	registers reg;									// The registers
	uint8_t   data;									// Last fetched byte
	uint8_t   x;									// Decoded opcode
	uint8_t   y;
	uint8_t   z;
	uint8_t   p;
	uint8_t   q;
	uint8_t	  shift_EXT;							// Shift into extended opcodes (CB,ED)
	uint8_t	  shift_IXY;							// Use registers 0=HL, 1=IX, 2=IY - includes undocumented IXL,IXY, IYL and IYH registers
	uint16_t  index_CB;								// The index (used for DDCB and FDCB instructions)
	uint8_t	  interrupt;
	int16_t   callDepth;

	void 		execute_CB();						// Execute CB prefixed opcodes
	void		execute_ED();						// Execute ED prefixed opcodes

	void		execute_trap();						// Unimplemented function

	void		execute_x0z0();						// Relative jumps and assorted ops
	void		execute_x0z1();						// 16 - bit load immediate / add
	void		execute_x0z2();						// Indirect load
	void		execute_x0z3();						// 16-bit increment/decrement
	void		execute_x0z4();						// 8-bit increment
	void		execute_x0z5();						// 8-bit decrement
	void		execute_x0z6();						// 8-bit load immediate
	void		execute_x0z7();						// Assorted operations on accumulator flags / LD (IX/Y+n),rr / LD rr, (IX/Y+n)

	void		execute_x1__();						// 8 bit loading
	void		execute_x2__();						// Operations on accumulator and register/memory location

	void		execute_x3z0();						// Conditional return
	void		execute_x3z1();						// POP and various operations#
	void		execute_x3z2();						// Conditional jump
	void		execute_x3z3();						// Assorted operations
	void		execute_x3z4();						// Conditional call
	void		execute_x3z5();						// PUSH and various operations
	void		execute_x3z6();						// Operate on accumulator and immediate operand
	void		execute_x3z7();						// Restart instructions

	bool		isROM(uint8_t* p);					// Return true if the pointer is in ROM space
	void		write(uint16_t a, uint8_t d);		// Write a byte to the address space
	void		write(uint8_t* p, uint8_t d);		// Write a byte to the address space by pointer
	uint8_t		read(uint16_t a);					// Read a byte from the address space
	void		push(uint16_t v);					// PUSH a value onto the stack
	uint16_t	pop();								// POP a value off the stack
	uint8_t*	getIndPtr(uint8_t s);				// Get indirect address pointer from HL, IX or IY
	uint8_t* 	getIXYPtr(uint8_t s, uint8_t d);	// Get indirect address pointer from IX+d or IY+d
	void		setFlagsSZP(uint8_t d);				// Set flags SZP based upon D

	void		out(uint16_t addr, uint8_t v);
	uint8_t		in(uint16_t addr);

	void		rlc(uint8_t* r);
	void		rrc(uint8_t* r);
	void		rl(uint8_t* r);
	void		rr(uint8_t* r);
	void		sla(uint8_t* r);
	void		sra(uint8_t* r);
	void		sll(uint8_t* r);
	void		srl(uint8_t* r);
	void		rlca(uint8_t* r);
	void		rrca(uint8_t* r);
	void		rla(uint8_t* r);
	void		rra(uint8_t* r);

	void 		ldi();
	void		cpi();
	void		ini();
	void		outi();
	void		ldd();
	void		cpd();
	void		ind();
	void		outd();
	void		ldir();
	void		cpir();
	void		inir();
	void		otir();
	void		lddr();
	void		cpdr();
	void		indr();
	void		otdr();
};