#include "registers.h"

registers::registers() :
	PC(0),
	IFF1(false),
	IFF2(false)
{
}

void registers::ex(REG* rp1, REG* rp2)
{
	uint16_t d = (*rp1).W;
	(*rp1).W = (*rp2).W;
	(*rp2).W = d;
}

void registers::A_add(uint8_t d) {
	uint16_t w = AF.A + d;
	uint8_t  b = w & 0xFF;
	AF.C = (w >= 0xFF);
	AF.S = (b >= 0x7F);
	AF.Z = (b == 0x00);
	AF.A = b;

}

void registers::A_adc(uint8_t d) {
	uint16_t w = AF.A + d + AF.C;
	uint8_t  b = w & 0xFF;
	AF.C = (w >= 0xFF);
	AF.S = (b >= 0x7F);
	AF.Z = (b == 0x00);
	AF.A = b;
}

void registers::A_sub(uint8_t d) {
	uint16_t w = AF.A - d;
	uint8_t  b = w & 0xFF;
	AF.C = (w >= 0xFF);
	AF.S = (b >= 0x7F);
	AF.Z = (b == 0x00);
	AF.A = b;
}

void registers::A_sbc(uint8_t d) {
	uint16_t w = AF.A - d - AF.C;
	uint8_t  b = w & 0xFF;
	AF.C = (w >= 0xFF);
	AF.S = (b >= 0x7F);
	AF.Z = (b == 0x00);
	AF.A = b;
}

void registers::A_and(uint8_t d) {
	AF.L = 0;
	AF.A &= d;
	AF.Z = (AF.A == 0x00);
	AF.S = (AF.A >= 0x7F);
}

void registers::A_xor(uint8_t d) {
	AF.L = 0;
	AF.A ^= d;
	AF.Z = (AF.A == 0x00);
	AF.S = (AF.A >= 0x7F);
}
void registers::A_or(uint8_t d) {
	AF.L = 0;
	AF.A |= d;
	AF.Z = (AF.A == 0x00);
	AF.S = (AF.A >= 0x7F);
}

void registers::A_cp(uint8_t d) {
	uint16_t w = AF.A - d;
	uint8_t  b = w & 0xFF;
	AF.C = (w >= 0xFF);
	AF.S = (b >= 0x7F);
	AF.Z = (b == 0x00);
}

void registers::A_neg() {
	AF.C = (AF.A != 0x00);
	AF.A=-AF.A;
	AF.S = (AF.A >= 0x7F);
	AF.Z = (AF.A == 0x00);
}

void registers::A_not() {
	AF.A=~AF.A;
}

void registers::A_daa() {
}

bool registers::F_NZ() { return AF.Z == 0; }
bool registers::F_Z()  { return AF.Z == 1; }
bool registers::F_NC() { return AF.C == 0; }
bool registers::F_C()  { return AF.C == 1; }
bool registers::F_PO() { return AF.P == 0; }
bool registers::F_PE() { return AF.P == 1; }
bool registers::F_P()  { return AF.S == 0; }
bool registers::F_M()  { return AF.S == 1; }