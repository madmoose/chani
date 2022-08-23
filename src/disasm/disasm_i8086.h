#ifndef DISASM_I8086
#define DISASM_I8086

#include "emu/emu.h"
#include "support/types.h"
#include "support/strbuf.h"

#include <optional>

class ibm5160_t;
class i8086_t;

enum arg_type_e: byte {
	PARAM_INHERIT,
	PARAM_NONE,

	PARAM_1,
	PARAM_3,

	PARAM_AL,
	PARAM_CL,
	PARAM_DL,
	PARAM_BL,

	PARAM_AH,
	PARAM_CH,
	PARAM_DH,
	PARAM_BH,

	PARAM_AX,
	PARAM_CX,
	PARAM_DX,
	PARAM_BX,

	PARAM_SP,
	PARAM_BP,
	PARAM_SI,
	PARAM_DI,

	PARAM_ES,
	PARAM_CS,
	PARAM_SS,
	PARAM_DS,

	PARAM_REG8,
	PARAM_REG16,
	PARAM_SREG,

	PARAM_IMM8,
	PARAM_IMM16,

	PARAM_REL8,
	PARAM_REL16,

	PARAM_IMEM8,
	PARAM_IMEM16,
	PARAM_IMEM32, // Needs a better name

	PARAM_MEM8,
	PARAM_MEM16,
	PARAM_MEM32, // Needs a better name
	PARAM_RM8,
	PARAM_RM16,
};

enum arg_dir_e: byte {
	RO = 1, // Read only
	RW,     // Read/write
	WO,     // Write only
};

struct opcode_t {
	const char *mnemonic;
	byte        flags;
	arg_type_e  arg_1;
	arg_type_e  arg_2;
	arg_dir_e   arg_1_dir;
	arg_dir_e   arg_2_dir;
};

class instruction_info_t {
	uint8_t type:2;

	void set_is_control_transfer() {
		type = 0;
	}

	void set_is_call() {
		type = 1;
	}

	void set_is_unconditional_jump() {
		type = 2;
	}

	void set_is_conditional_jump() {
		type = 3;
	}

public:
	bool is_control_transfer() {
		return type > 0;
	}

	bool is_call() {
		return type == 1;
	}

	bool is_unconditional_jump() {
		return type == 2;
	}

	bool is_conditional_jump() {
		return type == 3;
	}
};

struct mem_ref_t {
	uint16_t seg;
	uint16_t ofs;
	uint32_t value;
	byte     width;
};

class names_t;

class disasm_i8086_t {
	i8086_t *m_cpu = nullptr;

	uint16_t m_cs;
	uint16_t m_ip;
	uint16_t m_len;

	// Decode fields
	byte m_sreg_ovr;
	bool m_flag_f2;
	bool m_flag_f3;
	bool m_flag_lock;
	byte m_op;
	byte m_modrm;
	opcode_t m_opcode;
	uint32_t m_imm[2];

	bool m_has_mem_arg;

	const names_t *names = nullptr;

	byte     read8(uint16_t seg, uint16_t ofs);
	uint16_t read16(uint16_t seg, uint16_t ofs);

	byte     fetch8();
	uint16_t fetch16();
	uint32_t fetch32();

	void decode_param(int n, arg_type_e arg);

	strbuf_t strbuf;

	const char *str_imm(uint16_t imm);
	const char *str_reg(byte reg, bool w);
	const char *str_sreg(byte reg);
	const char *str_sreg_ovr(byte sreg_ovr);
	void        str_param(int n, arg_type_e arg, bool show_mem_width);

	uint16_t read_sreg(byte sreg);
	byte     get_sreg_ovr(byte sreg_def);
	uint16_t read_sreg_ovr(byte sreg_def);

public:
	disasm_i8086_t() {};

	bool always_show_mem_width = false;
	read_cb_t read;

	void set_cpu(i8086_t *);
	void set_names(const names_t *);

	void decode(uint16_t cs, uint16_t ip);
	void disassemble(uint16_t cs, uint16_t *ip, const char **s = 0);

	bool has_mem_arg() {
		return m_has_mem_arg;
	}

	std::optional<mem_ref_t> get_mem_arg();
};

#endif
