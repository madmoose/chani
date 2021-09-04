#ifndef DISASM_I8086
#define DISASM_I8086

#include "emu/emu.h"
#include "support/types.h"
#include "support/strbuf.h"

class ibm5160_t;

enum arg_type_e {
	PARAM_INHERIT,
	PARAM_NONE,

	PARAM_1,
	PARAM_3,

	PARAM_AH,
	PARAM_AL,
	PARAM_AX,
	PARAM_BH,
	PARAM_BL,
	PARAM_BP,
	PARAM_BX,
	PARAM_CH,
	PARAM_CL,
	PARAM_CS,
	PARAM_CX,
	PARAM_DH,
	PARAM_DI,
	PARAM_DL,
	PARAM_DS,
	PARAM_DX,
	PARAM_ES,
	PARAM_SI,
	PARAM_SP,
	PARAM_SS,

	PARAM_REG8,
	PARAM_REG16,
	PARAM_SREG,

	PARAM_IMM8,
	PARAM_IMM16,

	PARAM_REL8,
	PARAM_REL16,

	PARAM_IMEM8,
	PARAM_IMEM16,
	PARAM_IMEM32,

	PARAM_MEM8,
	PARAM_MEM16,
	PARAM_MEM32,
	PARAM_RM8,
	PARAM_RM16,
};

class disasm_i8086_t {
	uint16_t cs;
	uint16_t ip;

	byte sreg_ovr;
	bool flag_f2;
	bool flag_f3;
	bool flag_lock;
	byte op;
	byte modrm;

	strbuf_t strbuf;

	byte     read8(uint16_t seg, uint16_t ofs);
	uint16_t read16(uint16_t seg, uint16_t ofs);

	byte     fetch8();
	uint16_t fetch16();

	const char *str_imm(uint16_t imm);
	const char *str_reg(byte reg, bool w);
	const char *str_sreg(byte reg);
	const char *str_sreg_ovr(byte sreg_ovr);

	void handle_param(arg_type_e arg);

public:
	read_cb_t  read;

	disasm_i8086_t() {};

	void disassemble(uint16_t cs, uint16_t *ip, const char **s = 0);
};

#endif
