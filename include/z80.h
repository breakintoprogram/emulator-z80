//
// Title:	        Z80 CPU
// Description:		Z80 CPU emulation
// Author:	        Dean Belfield
// Created:	        22/05/2026
// Last Updated:	28/05/2026
//
// Modinfo:

#pragma once

#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <algorithm>
#include <cstdint>

#include "defines.h"
#include "registers.h"
#include "mem.h"
#include "ports.h"
#include "logger.h"

using namespace std;

class Z80 {
public:
	Z80(Mem* mem, Ports* ports, Logger* logger);

	void     addBreakpoint(uint16_t a);
	bool     getSingleStep();
	void     setSingleStep(bool value);
	bool     getTrace();
	void     setTrace(bool value);
	uint32_t getT();

	void     dump(bool newline);

	void     interruptRequest();

	void     reset();
	void     run();

private:
	Mem*      mem;				// Pointer to the memory class
	Ports*    ports;			// Pointer to the ports class
	Logger*   logger;			// Pointer to the logger class
	Registers reg;				// The registers

	uint8_t	  data;				// Last fetched byte
	uint8_t	  x;				// Decoded opcode values
	uint8_t	  y;
	uint8_t	  z;
	uint8_t	  p;
	uint8_t	  q;
	uint32_t  t;				// T-state counter
	uint8_t	  shift_IXY;		// Use registers 0=HL, 1=IX, 2=IY - includes undocumented IXL,IXY, IYL and IYH registers
	bool	  interrupt;		// Set when an interrupt has been requested
	bool      halted;			// Is the CPU in a halt state?
	bool      blockop;			// Is the CPU executing a block operation?
	bool	  singleStep;		// Set when single-stepping
	bool	  trace;			// Set to enable tracing to traceStream

	vector<uint16_t> breakpoints;	// The breakpoint list

	// Look-up tables for the x and z decode stage
	//
	vector<vector<void (Z80::*)()>> lut_xz = {
		{ &Z80::execute_x0z0, &Z80::execute_x0z1, &Z80::execute_x0z2, &Z80::execute_x0z3, &Z80::execute_x0z4, &Z80::execute_x0z5, &Z80::execute_x0z6, &Z80::execute_x0z7 },
		{ &Z80::execute_x1__, &Z80::execute_x1__, &Z80::execute_x1__, &Z80::execute_x1__, &Z80::execute_x1__, &Z80::execute_x1__, &Z80::execute_x1__, &Z80::execute_x1__ },
		{ &Z80::execute_x2__, &Z80::execute_x2__, &Z80::execute_x2__, &Z80::execute_x2__, &Z80::execute_x2__, &Z80::execute_x2__, &Z80::execute_x2__, &Z80::execute_x2__ },
		{ &Z80::execute_x3z0, &Z80::execute_x3z1, &Z80::execute_x3z2, &Z80::execute_x3z3, &Z80::execute_x3z4, &Z80::execute_x3z5, &Z80::execute_x3z6, &Z80::execute_x3z7 }
	};

	// Look-up tables for ALU operations
	//
	vector<void (Registers::*)(uint8_t)> lut_alu1 = {
		&Registers::adda,
		&Registers::adca,
		&Registers::suba,
		&Registers::sbca,
		&Registers::anda,
		&Registers::xora,
		&Registers::ora,
		&Registers::cpa
	};

	vector<void (Registers::*)()> lut_alu2 = {
		&Registers::rlca,
		&Registers::rrca,
		&Registers::rla,
		&Registers::rra,
		&Registers::daa,
		&Registers::cpl,
		&Registers::scf,
		&Registers::ccf
	};

	// Look-up table for ROT operations
	//
	vector<void (Registers::*)(uint8_t *)> lut_rot = {
		&Registers::rlc,
		&Registers::rrc,
		&Registers::rl,
		&Registers::rr,
		&Registers::sla,
		&Registers::sra,
		&Registers::sll,
		&Registers::srl
	};

	// Look-up table for block operations
	//
	vector<vector<void (Z80::*)()>> lut_bli = {
		{ &Z80::ldi,  &Z80::cpi,  &Z80::ini,  &Z80::outi },
		{ &Z80::ldd,  &Z80::cpd,  &Z80::ind,  &Z80::outd },
		{ &Z80::ldir, &Z80::cpir, &Z80::inir, &Z80::otir },
		{ &Z80::lddr, &Z80::cpdr, &Z80::indr, &Z80::otdr }
	};

	// Look-up table for conditions
	//
	vector<bool (Registers::*)()> lut_cc = {
		&Registers::F_NZ,
		&Registers::F_Z,
		&Registers::F_NC,
		&Registers::F_C,
		&Registers::F_PO,
		&Registers::F_PE,
		&Registers::F_P,
		&Registers::F_M,
	};

	// 8-bit register lookup
	//
	uint8_t* t_r[3][8] = {
		{ &reg.BC.H, &reg.BC.L, &reg.DE.H, &reg.DE.L, &reg.HL.H, &reg.HL.L, NULL, &reg.AF.A },	// NULL is (HL)
		{ &reg.BC.H, &reg.BC.L, &reg.DE.H, &reg.DE.L, &reg.IX.H, &reg.IX.L, NULL, &reg.AF.A },	// NULL is (IX)
		{ &reg.BC.H, &reg.BC.L, &reg.DE.H, &reg.DE.L, &reg.IY.H, &reg.IY.L, NULL, &reg.AF.A }	// NULL is (IY)
	};

	// 16-bit register lookups
	//
	uint16_t * t_rp1[3][4] = {
		{ &reg.BC.W, &reg.DE.W, &reg.HL.W, &reg.SP   },
		{ &reg.BC.W, &reg.DE.W, &reg.IX.W, &reg.SP   },
		{ &reg.BC.W, &reg.DE.W, &reg.IY.W, &reg.SP   }
	};
	
	uint16_t * t_rp2[3][4] = {
		{ &reg.BC.W, &reg.DE.W, &reg.HL.W, &reg.AF.W },
		{ &reg.BC.W, &reg.DE.W, &reg.IX.W, &reg.AF.W },
		{ &reg.BC.W, &reg.DE.W, &reg.IY.W, &reg.AF.W }
	};

	void        debug();
	void		interrupts();
	void        fetch();
	void        decode();
	void        execute();
	void		setT(uint16_t value);
	void		incT(uint16_t value);

	void 		execute_CB();					// Execute CB prefixed opcodes
	void		execute_ED();					// Execute ED prefixed opcodes
	void		execute_x0z0();					// Relative jumps and assorted ops
	void		execute_x0z1();					// 16 - bit load immediate / add
	void		execute_x0z2();					// Indirect load
	void		execute_x0z3();					// 16-bit increment/decrement
	void		execute_x0z4();					// 8-bit increment
	void		execute_x0z5();					// 8-bit decrement
	void		execute_x0z6();					// 8-bit load immediate
	void		execute_x0z7();					// Assorted operations on accumulator flags / LD (IX/Y+n),rr / LD rr, (IX/Y+n)
	void		execute_x1__();					// 8 bit loading
	void		execute_x2__();					// Operations on accumulator and register/memory location
	void		execute_x3z0();					// Conditional return
	void		execute_x3z1();					// POP and various operations#
	void		execute_x3z2();					// Conditional jump
	void		execute_x3z3();					// Assorted operations
	void		execute_x3z4();					// Conditional call
	void		execute_x3z5();					// PUSH and various operations
	void		execute_x3z6();					// Operate on accumulator and immediate operand
	void		execute_x3z7();					// Restart instructions
	uint16_t	fetchWord();					// Fetch a word from the PC
	void		push(uint16_t v);				// PUSH a value onto the stack
	uint16_t	pop();							// POP a value off the stack
	uint16_t	getInd(uint8_t s);				// Get indirect address from HL, IX or IY
	uint16_t 	getIXY(uint8_t s, uint8_t d);	// Get indirect address from IX+d or IY+d

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
