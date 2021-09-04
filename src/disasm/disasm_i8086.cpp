#include "disasm/disasm_i8086.h"

enum {
	NONE,
	MODRM,
	GROUP,
};

struct opcode_t {
	const char *mnemonic;
	byte        flags;
	arg_type_e  arg_1;
	arg_type_e  arg_2;
};

/*
 * Checked against http://mlsite.net/8086/
 */
const opcode_t i8086_opcode_table[256] = {
	/* 00 */ { "add",    MODRM, PARAM_RM8,    PARAM_REG8   },
	/* 01 */ { "add",    MODRM, PARAM_RM16,   PARAM_REG16  },
	/* 02 */ { "add",    MODRM, PARAM_REG8,   PARAM_RM8    },
	/* 03 */ { "add",    MODRM, PARAM_REG16,  PARAM_RM16   },
	/* 04 */ { "add",    0,     PARAM_IMM8                 },
	/* 05 */ { "add",    0,     PARAM_IMM16                },
	/* 06 */ { "push",   0,     PARAM_ES                   },
	/* 07 */ { "pop",    0,     PARAM_ES                   },
	/* 08 */ { "or",     MODRM, PARAM_RM8,    PARAM_REG8   },
	/* 09 */ { "or",     MODRM, PARAM_RM16,   PARAM_REG16  },
	/* 0a */ { "or",     MODRM, PARAM_REG8,   PARAM_RM8    },
	/* 0b */ { "or",     MODRM, PARAM_REG16,  PARAM_RM16   },
	/* 0c */ { "or",     0,     PARAM_IMM8                 },
	/* 0d */ { "or",     0,     PARAM_IMM16                },
	/* 0e */ { "push",   0,     PARAM_CS                   },
	/* 0f */ { "pop",    0,     PARAM_CS                   },
	/* 10 */ { "adc",    MODRM, PARAM_RM8,    PARAM_REG8   },
	/* 11 */ { "adc",    MODRM, PARAM_RM16,   PARAM_REG16  },
	/* 12 */ { "adc",    MODRM, PARAM_REG8,   PARAM_RM8    },
	/* 13 */ { "adc",    MODRM, PARAM_REG16,  PARAM_RM16   },
	/* 14 */ { "adc",    0,     PARAM_IMM8                 },
	/* 15 */ { "adc",    0,     PARAM_IMM16                },
	/* 16 */ { "push",   0,     PARAM_SS                   },
	/* 17 */ { "pop",    0,     PARAM_SS                   },
	/* 18 */ { "sbb",    MODRM, PARAM_RM8,    PARAM_REG8   },
	/* 19 */ { "sbb",    MODRM, PARAM_RM16,   PARAM_REG16  },
	/* 1a */ { "sbb",    MODRM, PARAM_REG8,   PARAM_RM8    },
	/* 1b */ { "sbb",    MODRM, PARAM_REG16,  PARAM_RM16   },
	/* 1c */ { "sbb",    0,     PARAM_IMM8                 },
	/* 1d */ { "sbb",    0,     PARAM_IMM16                },
	/* 1e */ { "push",   0,     PARAM_DS                   },
	/* 1f */ { "pop",    0,     PARAM_DS                   },
	/* 20 */ { "and",    MODRM, PARAM_RM8,    PARAM_REG8   },
	/* 21 */ { "and",    MODRM, PARAM_RM16,   PARAM_REG16  },
	/* 22 */ { "and",    MODRM, PARAM_REG8,   PARAM_RM8    },
	/* 23 */ { "and",    MODRM, PARAM_REG16,  PARAM_RM16   },
	/* 24 */ { "and",    0,     PARAM_IMM8                 },
	/* 25 */ { "and",    0,     PARAM_IMM16                },
	/* 26 */ {                                             },
	/* 27 */ { "daa"                                       },
	/* 28 */ { "sub",    MODRM, PARAM_RM8,    PARAM_REG8   },
	/* 29 */ { "sub",    MODRM, PARAM_RM16,   PARAM_REG16  },
	/* 2a */ { "sub",    MODRM, PARAM_REG8,   PARAM_RM8    },
	/* 2b */ { "sub",    MODRM, PARAM_REG16,  PARAM_RM16   },
	/* 2c */ { "sub",    0,     PARAM_IMM8                 },
	/* 2d */ { "sub",    0,     PARAM_IMM16                },
	/* 2e */ {                                             },
	/* 2f */ { "das"                                       },
	/* 30 */ { "xor",    MODRM, PARAM_RM8,    PARAM_REG8   },
	/* 31 */ { "xor",    MODRM, PARAM_RM16,   PARAM_REG16  },
	/* 32 */ { "xor",    MODRM, PARAM_REG8,   PARAM_RM8    },
	/* 33 */ { "xor",    MODRM, PARAM_REG16,  PARAM_RM16   },
	/* 34 */ { "xor",    0,     PARAM_IMM8                 },
	/* 35 */ { "xor",    0,     PARAM_IMM16                },
	/* 36 */ {                                             },
	/* 37 */ { "aaa"                                       },
	/* 38 */ { "cmp",    MODRM, PARAM_RM8,    PARAM_REG8   },
	/* 39 */ { "cmp",    MODRM, PARAM_RM16,   PARAM_REG16  },
	/* 3a */ { "cmp",    MODRM, PARAM_REG8,   PARAM_RM8    },
	/* 3b */ { "cmp",    MODRM, PARAM_REG16,  PARAM_RM16   },
	/* 3c */ { "cmp",    0,     PARAM_IMM8                 },
	/* 3d */ { "cmp",    0,     PARAM_IMM16                },
	/* 3e */ {                                             },
	/* 3f */ { "aas"                                       },
	/* 40 */ { "inc",    0,     PARAM_AX                   },
	/* 41 */ { "inc",    0,     PARAM_CX                   },
	/* 42 */ { "inc",    0,     PARAM_DX                   },
	/* 43 */ { "inc",    0,     PARAM_BX                   },
	/* 44 */ { "inc",    0,     PARAM_SP                   },
	/* 45 */ { "inc",    0,     PARAM_BP                   },
	/* 46 */ { "inc",    0,     PARAM_SI                   },
	/* 47 */ { "inc",    0,     PARAM_DI                   },
	/* 48 */ { "dec",    0,     PARAM_AX                   },
	/* 49 */ { "dec",    0,     PARAM_CX                   },
	/* 4a */ { "dec",    0,     PARAM_DX                   },
	/* 4b */ { "dec",    0,     PARAM_BX                   },
	/* 4c */ { "dec",    0,     PARAM_SP                   },
	/* 4d */ { "dec",    0,     PARAM_BP                   },
	/* 4e */ { "dec",    0,     PARAM_SI                   },
	/* 4f */ { "dec",    0,     PARAM_DI                   },
	/* 50 */ { "push",   0,     PARAM_AX                   },
	/* 51 */ { "push",   0,     PARAM_CX                   },
	/* 52 */ { "push",   0,     PARAM_DX                   },
	/* 53 */ { "push",   0,     PARAM_BX                   },
	/* 54 */ { "push",   0,     PARAM_SP                   },
	/* 55 */ { "push",   0,     PARAM_BP                   },
	/* 56 */ { "push",   0,     PARAM_SI                   },
	/* 57 */ { "push",   0,     PARAM_DI                   },
	/* 58 */ { "pop",    0,     PARAM_AX                   },
	/* 59 */ { "pop",    0,     PARAM_CX                   },
	/* 5a */ { "pop",    0,     PARAM_DX                   },
	/* 5b */ { "pop",    0,     PARAM_BX                   },
	/* 5c */ { "pop",    0,     PARAM_SP                   },
	/* 5d */ { "pop",    0,     PARAM_BP                   },
	/* 5e */ { "pop",    0,     PARAM_SI                   },
	/* 5f */ { "pop",    0,     PARAM_DI                   },
	/* 60 */ {                                             },
	/* 61 */ {                                             },
	/* 62 */ {                                             },
	/* 63 */ {                                             },
	/* 64 */ {                                             },
	/* 65 */ {                                             },
	/* 66 */ {                                             },
	/* 67 */ {                                             },
	/* 68 */ {                                             },
	/* 69 */ {                                             },
	/* 6a */ {                                             },
	/* 6b */ {                                             },
	/* 6c */ {                                             },
	/* 6d */ {                                             },
	/* 6e */ {                                             },
	/* 6f */ {                                             },
	/* 70 */ { "jo",     0,     PARAM_REL8                 },
	/* 71 */ { "jno",    0,     PARAM_REL8                 },
	/* 72 */ { "jb",     0,     PARAM_REL8                 },
	/* 73 */ { "jnb",    0,     PARAM_REL8                 },
	/* 74 */ { "jz",     0,     PARAM_REL8                 },
	/* 75 */ { "jnz",    0,     PARAM_REL8                 },
	/* 76 */ { "jbe",    0,     PARAM_REL8                 },
	/* 77 */ { "ja",     0,     PARAM_REL8                 },
	/* 78 */ { "js",     0,     PARAM_REL8                 },
	/* 79 */ { "jns",    0,     PARAM_REL8                 },
	/* 7a */ { "jpe",    0,     PARAM_REL8                 },
	/* 7b */ { "jpo",    0,     PARAM_REL8                 },
	/* 7c */ { "jl",     0,     PARAM_REL8                 },
	/* 7d */ { "jge",    0,     PARAM_REL8                 },
	/* 7e */ { "jle",    0,     PARAM_REL8                 },
	/* 7f */ { "jg",     0,     PARAM_REL8                 },
	/* 80 */ { 0,        GROUP, PARAM_RM8,    PARAM_IMM8   },
	/* 81 */ { 0,        GROUP, PARAM_RM16,   PARAM_IMM16  },
	/* 82 */ { 0,        GROUP, PARAM_RM8,    PARAM_IMM8   },
	/* 83 */ { 0,        GROUP, PARAM_RM16,   PARAM_IMM8   },
	/* 84 */ { "test",   MODRM, PARAM_REG8,   PARAM_RM8    },
	/* 85 */ { "test",   MODRM, PARAM_REG16,  PARAM_RM16   },
	/* 86 */ { "xchg",   MODRM, PARAM_REG8,   PARAM_RM8    },
	/* 87 */ { "xchg",   MODRM, PARAM_REG16,  PARAM_RM16   },
	/* 88 */ { "mov",    MODRM, PARAM_RM8,    PARAM_REG8   },
	/* 89 */ { "mov",    MODRM, PARAM_RM16,   PARAM_REG16  },
	/* 8a */ { "mov",    MODRM, PARAM_REG8,   PARAM_RM8    },
	/* 8b */ { "mov",    MODRM, PARAM_REG16,  PARAM_RM16   },
	/* 8c */ { "mov",    MODRM, PARAM_RM16,   PARAM_SREG   },
	/* 8d */ { "lea",    MODRM, PARAM_REG16,  PARAM_MEM16  },
	/* 8e */ { "mov",    MODRM, PARAM_SREG,   PARAM_RM16   },
	/* 8f */ { "pop",    MODRM, PARAM_RM16                 },
	/* 90 */ { "nop"                                       },
	/* 91 */ { "xchg",   0,     PARAM_CX,     PARAM_AX     },
	/* 92 */ { "xchg",   0,     PARAM_DX,     PARAM_AX     },
	/* 93 */ { "xchg",   0,     PARAM_BX,     PARAM_AX     },
	/* 94 */ { "xchg",   0,     PARAM_SP,     PARAM_AX     },
	/* 95 */ { "xchg",   0,     PARAM_BP,     PARAM_AX     },
	/* 96 */ { "xchg",   0,     PARAM_SI,     PARAM_AX     },
	/* 97 */ { "xchg",   0,     PARAM_DI,     PARAM_AX     },
	/* 98 */ { "cbw"                                       },
	/* 99 */ { "cwd"                                       },
	/* 9a */ { "call",   0,     PARAM_IMEM32               },
	/* 9b */ { "wait"                                      },
	/* 9c */ { "pushf"                                     },
	/* 9d */ { "popf"                                      },
	/* 9e */ { "sahf"                                      },
	/* 9f */ { "lahf"                                      },
	/* a0 */ { "mov",    0,     PARAM_AL,     PARAM_IMEM8  },
	/* a1 */ { "mov",    0,     PARAM_AX,     PARAM_IMEM16 },
	/* a2 */ { "mov",    0,     PARAM_IMEM8,  PARAM_AL     },
	/* a3 */ { "mov",    0,     PARAM_IMEM16, PARAM_AX     },
	/* a4 */ { "movsb"                                     },
	/* a5 */ { "movsw"                                     },
	/* a6 */ { "cmpsb"                                     },
	/* a7 */ { "cmpsw"                                     },
	/* a8 */ { "test",   0,     PARAM_AL,     PARAM_IMM8   },
	/* a9 */ { "test",   0,     PARAM_AX,     PARAM_IMM16  },
	/* aa */ { "stosb"                                     },
	/* ab */ { "stosw"                                     },
	/* ac */ { "lodsb"                                     },
	/* ad */ { "lodsw"                                     },
	/* ae */ { "scasb"                                     },
	/* af */ { "scasw"                                     },
	/* b0 */ { "mov",    0,     PARAM_AL,     PARAM_IMM8   },
	/* b1 */ { "mov",    0,     PARAM_CL,     PARAM_IMM8   },
	/* b2 */ { "mov",    0,     PARAM_DL,     PARAM_IMM8   },
	/* b3 */ { "mov",    0,     PARAM_BL,     PARAM_IMM8   },
	/* b4 */ { "mov",    0,     PARAM_AH,     PARAM_IMM8   },
	/* b5 */ { "mov",    0,     PARAM_CH,     PARAM_IMM8   },
	/* b6 */ { "mov",    0,     PARAM_DH,     PARAM_IMM8   },
	/* b7 */ { "mov",    0,     PARAM_BH,     PARAM_IMM8   },
	/* b8 */ { "mov",    0,     PARAM_AX,     PARAM_IMM16  },
	/* b9 */ { "mov",    0,     PARAM_CX,     PARAM_IMM16  },
	/* ba */ { "mov",    0,     PARAM_DX,     PARAM_IMM16  },
	/* bb */ { "mov",    0,     PARAM_BX,     PARAM_IMM16  },
	/* bc */ { "mov",    0,     PARAM_SP,     PARAM_IMM16  },
	/* bd */ { "mov",    0,     PARAM_BP,     PARAM_IMM16  },
	/* be */ { "mov",    0,     PARAM_SI,     PARAM_IMM16  },
	/* bf */ { "mov",    0,     PARAM_DI,     PARAM_IMM16  },
	/* c0 */ {                                             },
	/* c1 */ {                                             },
	/* c2 */ { "ret",    0,     PARAM_IMM16                },
	/* c3 */ { "ret"                                       },
	/* c4 */ { "les",    MODRM, PARAM_REG16,  PARAM_MEM32  },
	/* c5 */ { "lds",    MODRM, PARAM_REG16,  PARAM_MEM32  },
	/* c6 */ { "mov",    MODRM, PARAM_RM8,    PARAM_IMM8   },
	/* c7 */ { "mov",    MODRM, PARAM_RM16,   PARAM_IMM16  },
	/* c8 */ {                                             },
	/* c9 */ {                                             },
	/* ca */ { "retf",   0,     PARAM_IMM16                },
	/* cb */ { "retf"                                      },
	/* cc */ { "int",    0,     PARAM_3                    },
	/* cd */ { "int",    0,     PARAM_IMM8                 },
	/* ce */ { "into"                                      },
	/* cf */ { "iret"                                      },
	/* d0 */ { 0,        GROUP, PARAM_RM8,    PARAM_1      },
	/* d1 */ { 0,        GROUP, PARAM_RM16,   PARAM_1      },
	/* d2 */ { 0,        GROUP, PARAM_RM8,    PARAM_CL     },
	/* d3 */ { 0,        GROUP, PARAM_RM16,   PARAM_CL     },
	/* d4 */ { "aam",    0,     PARAM_IMM8                 },
	/* d5 */ { "aad",    0,     PARAM_IMM8                 },
	/* d6 */ {                                             },
	/* d7 */ { "xlat"                                      },
	/* d8 */ {                                             },
	/* d9 */ {                                             },
	/* da */ {                                             },
	/* db */ {                                             },
	/* dc */ {                                             },
	/* dd */ {                                             },
	/* de */ {                                             },
	/* df */ {                                             },
	/* e0 */ { "loopnz", 0,     PARAM_REL8                 },
	/* e1 */ { "loopz",  0,     PARAM_REL8                 },
	/* e2 */ { "loop",   0,     PARAM_REL8                 },
	/* e3 */ { "jcxz",   0,     PARAM_REL8                 },
	/* e4 */ { "in",     0,     PARAM_AL,     PARAM_IMM8   },
	/* e5 */ { "in",     0,     PARAM_AX,     PARAM_IMM8   },
	/* e6 */ { "out",    0,     PARAM_IMM8,   PARAM_AL     },
	/* e7 */ { "out",    0,     PARAM_IMM8,   PARAM_AX     },
	/* e8 */ { "call",   0,     PARAM_REL16                },
	/* e9 */ { "jmp",    0,     PARAM_REL16                },
	/* ea */ { "jmp",    0,     PARAM_IMEM32               },
	/* eb */ { "jmp",    0,     PARAM_REL8                 },
	/* ec */ { "in",     0,     PARAM_AL,     PARAM_DX     },
	/* ed */ { "in",     0,     PARAM_AX,     PARAM_DX     },
	/* ee */ { "out",    0,     PARAM_DX,     PARAM_AL     },
	/* ef */ { "out",    0,     PARAM_DX,     PARAM_AX     },
	/* f0 */ { "lock"                                      },
	/* f1 */ {                                             },
	/* f2 */ {                                             },
	/* f3 */ {                                             },
	/* f4 */ { "hlt"                                       },
	/* f5 */ { "cmc"                                       },
	/* f6 */ { 0,        GROUP, PARAM_RM8,    PARAM_IMM8   },
	/* f7 */ { 0,        GROUP, PARAM_RM16,   PARAM_IMM16  },
	/* f8 */ { "clc"                                       },
	/* f9 */ { "stc"                                       },
	/* fa */ { "cli"                                       },
	/* fb */ { "sti"                                       },
	/* fc */ { "cld"                                       },
	/* fd */ { "std"                                       },
	/* fe */ { 0,        GROUP, PARAM_RM8                  },
	/* ff */ { 0,        GROUP, PARAM_RM16                 },
};

const opcode_t i8086_opcode_table_grp_1[8] = {
	{ "add" }, { "or"  }, { "adc" }, { "sbb" }, { "and" }, { "sub" }, { "xor" }, { "cmp" },
};

const opcode_t i8086_opcode_table_grp_ff[8] = {
	{ "inc",  0, PARAM_RM8  },
	{ "dec",  0, PARAM_RM8  },
	{ "call", 0, PARAM_RM8  },
	{ "call", 0, PARAM_RM16 },
	{ "jmp",  0, PARAM_RM8  },
	{ "jmp",  0, PARAM_RM16 },
	{ "push" },
};

const opcode_t i8086_opcode_table_grp_2[8] = {
	{ "rol" }, { "ror" }, { "rcl" }, { "rcr" }, { "shl" }, { "shr" }, { "sal" }, { "sar" },
};

const opcode_t i8086_opcode_table_grp_3[8] = {
	{"test" },
	{"test" },
	{"not",  0, PARAM_INHERIT, PARAM_NONE },
	{"neg",  0, PARAM_INHERIT, PARAM_NONE },
	{"mul",  0, PARAM_INHERIT, PARAM_NONE },
	{"imul", 0, PARAM_INHERIT, PARAM_NONE },
	{"div",  0, PARAM_INHERIT, PARAM_NONE },
	{"idiv", 0, PARAM_INHERIT, PARAM_NONE },
};

const opcode_t i8086_opcode_table_grp_4[8] = {
	{ "inc" }, { "dec" },
};

const opcode_t i8086_opcode_table_grp_5[8] = {
	{ "inc"               },
	{ "dec"               },
	{ "call"              },
	{ "call", PARAM_MEM32 },
	{ "jmp"               },
	{ "jmp",  PARAM_MEM32 },
	{ "push"              },
	{ 0 },
};

byte disasm_i8086_t::read8(uint16_t seg, uint16_t ofs) {
	uint32_t ea = 0x10 * seg + ofs;
	byte v = read(MEM, ea, W8);
	return v;
}

uint16_t disasm_i8086_t::read16(uint16_t seg, uint16_t ofs) {
	uint32_t ea = 0x10 * seg + ofs;
	uint16_t v = read(MEM, ea, W16);
	return v;
}

byte disasm_i8086_t::fetch8() {
	byte v = read8(cs, ip);
	ip += 1;
	return v;
}

uint16_t disasm_i8086_t::fetch16() {
	uint16_t v = read16(cs, ip);
	ip += 2;
	return v;
}

static bool needs_modrm(const opcode_t &opcode) {
	return opcode.flags == MODRM || opcode.flags == GROUP;
}

static opcode_t inherit(const opcode_t op_detail, const opcode_t grp) {
	return opcode_t {
		.mnemonic = op_detail.mnemonic,
		.flags = op_detail.flags,
		.arg_1 = op_detail.arg_1 ? op_detail.arg_1 : grp.arg_1,
		.arg_2 = op_detail.arg_2 ? op_detail.arg_2 : grp.arg_2,
	};
}

static bool is_mem_arg(arg_type_e arg, byte modrm) {
	switch (arg) {
		case PARAM_IMEM8:
		case PARAM_IMEM16:
		case PARAM_IMEM32:
		case PARAM_MEM8:
		case PARAM_MEM16:
		case PARAM_MEM32:
			return true;
		case PARAM_RM8:
		case PARAM_RM16:
			return modrm < 0xc0;
		default:;
	}
	return false;
}

void disasm_i8086_t::disassemble(uint16_t a_cs, uint16_t *a_ip, const char **s) {
	cs = a_cs;
	ip = *a_ip;

	sreg_ovr  = 0;
	flag_f2   = false;
	flag_f3   = false;
	flag_lock = false;
	op        = 0;
	modrm     = 0;
	sreg_ovr  = 0;

	strbuf.clear();

	for (;;) {
		op = fetch8();

		// Handle prefixes
		switch (op) {
			case 0x2e: sreg_ovr = op; break; // CS
			case 0x36: sreg_ovr = op; break; // SS
			case 0x3e: sreg_ovr = op; break; // DS
			case 0x26: sreg_ovr = op; break; // ES
			case 0xf0: // LOCK
				flag_lock = true;
				break;
			case 0xf2: // REPNE
				flag_f2 = true;
				flag_f3 = false;
				break;
			case 0xf3: // REP
				flag_f3 = true;
				flag_f2 = false;
				break;
			default:
				goto opcode;
		}
	}
opcode:

	opcode_t opcode = i8086_opcode_table[op];

	if (needs_modrm(opcode)) {
		modrm = fetch8();
	}

#define REG ((modrm >> 3) & 0b111)

	switch (op) {
		case 0x80: case 0x81: case 0x82: case 0x83:
			opcode = inherit(i8086_opcode_table_grp_1[REG], opcode); break;
		case 0xd0: case 0xd1: case 0xd2: case 0xd3:
			opcode = inherit(i8086_opcode_table_grp_2[REG], opcode); break;
		case 0xf6: case 0xf7:
			opcode = inherit(i8086_opcode_table_grp_3[REG], opcode); break;
		case 0xfe:
			opcode = inherit(i8086_opcode_table_grp_4[REG], opcode); break;
		case 0xff:
			opcode = inherit(i8086_opcode_table_grp_5[REG], opcode); break;
	}

#undef REG

	strbuf.sprintf("%04x:%04x\t", cs, ip);
	strbuf.sprintf("%02x\t", op);

	if (!opcode.mnemonic) {
		strbuf.sprintf("db\t%s", str_imm(op));
	} else {
		if (flag_lock) {
			strbuf.sprintf("lock ");
		}
		if (flag_f2) {
			switch (op) {
				case 0xa6: // cmpsb
				case 0xa7: // cmpsw
					strbuf.sprintf("repne ");
					break;
				case 0xae: // scasb
				case 0xaf: // scasw
					strbuf.sprintf("repne ");
					break;
			}
		} else if (flag_f3) {
			switch (op) {
				case 0xa4: // movsb
				case 0xa5: // movsw
					strbuf.sprintf("rep ");
					break;
				case 0xa6: // cmpsb
				case 0xa7: // cmpsw
					strbuf.sprintf("repe ");
					break;
				case 0xaa: // stosb
				case 0xab: // stosw
					strbuf.sprintf("rep ");
					break;
				case 0xac: // lodsb
				case 0xad: // lodsw
					strbuf.sprintf("rep ");
					break;
				case 0xae: // scasb
				case 0xaf: // scasw
					strbuf.sprintf("repe ");
					break;
			}
		}

		if (sreg_ovr) {
			bool has_mem_arg = is_mem_arg(opcode.arg_1, modrm)
							|| is_mem_arg(opcode.arg_2, modrm);

			if (!has_mem_arg) {
				strbuf.sprintf("%s", str_sreg_ovr(sreg_ovr));
			}
		}

		strbuf.sprintf("%s", opcode.mnemonic);

		if (opcode.arg_1) {
			strbuf.sprintf("\t");
			handle_param(opcode.arg_1);
		}
		if (opcode.arg_2) {
			strbuf.sprintf(", ");
			handle_param(opcode.arg_2);
		}
	}

	*a_ip = this->ip;
	if (s) {
		*s = strbuf.cstr();
	}
}

const char *disasm_i8086_t::str_imm(uint16_t imm) {
	static char s[16];

	uint16_t t = imm;
	while (t > 0xf) {
		t >>= 4;
	}

	sprintf(s, "%s%X%s", t > 9 ? "0" : "", imm, imm < 10 ? "" : "h");
	return s;
}

const char *disasm_i8086_t::str_reg(byte reg, bool w) {
	const char *regnames = "al\0cl\0dl\0bl\0ah\0ch\0dh\0bh\0ax\0cx\0dx\0bx\0sp\0bp\0si\0di";
	byte index = (((byte)w << 3) | reg);
	return &regnames[3 * index];
}

const char *disasm_i8086_t::str_sreg(byte reg) {
	const char *regnames = "es\0cs\0ss\0ds";
	return &regnames[3 * reg];
}

const char *disasm_i8086_t::str_sreg_ovr(byte sreg_ovr) {
	switch (sreg_ovr) {
		case 0x2e: return "cs:";
		case 0x36: return "ss:";
		case 0x3e: return "ds:";
		case 0x26: return "es:";
	}
	return "";
}

void disasm_i8086_t::handle_param(arg_type_e arg) {
	switch (arg) {
	case PARAM_INHERIT:
	case PARAM_NONE:
		break;
	case PARAM_1:
		strbuf.sprintf("1");
		break;
	case PARAM_3:
		strbuf.sprintf("3");
		break;
	case PARAM_AH:
		strbuf.sprintf("ah");
		break;
	case PARAM_AL:
		strbuf.sprintf("al");
		break;
	case PARAM_AX:
		strbuf.sprintf("ax");
		break;
	case PARAM_BH:
		strbuf.sprintf("bh");
		break;
	case PARAM_BL:
		strbuf.sprintf("bl");
		break;
	case PARAM_BP:
		strbuf.sprintf("bp");
		break;
	case PARAM_BX:
		strbuf.sprintf("bx");
		break;
	case PARAM_CH:
		strbuf.sprintf("ch");
		break;
	case PARAM_CL:
		strbuf.sprintf("cl");
		break;
	case PARAM_CS:
		strbuf.sprintf("cs");
		break;
	case PARAM_CX:
		strbuf.sprintf("cx");
		break;
	case PARAM_DH:
		strbuf.sprintf("dh");
		break;
	case PARAM_DI:
		strbuf.sprintf("di");
		break;
	case PARAM_DL:
		strbuf.sprintf("dl");
		break;
	case PARAM_DS:
		strbuf.sprintf("ds");
		break;
	case PARAM_DX:
		strbuf.sprintf("dx");
		break;
	case PARAM_ES:
		strbuf.sprintf("es");
		break;
	case PARAM_IMM8:
		strbuf.sprintf("%s", str_imm(fetch8()));
		break;
	case PARAM_IMM16:
		strbuf.sprintf("%s", str_imm(fetch16()));
		break;
	case PARAM_IMEM8:
	case PARAM_IMEM16:
		strbuf.sprintf("%s[%s]", str_sreg_ovr(sreg_ovr), str_imm(fetch16()));
		break;
	case PARAM_IMEM32:
		{
			uint16_t ofs = fetch16();
			uint16_t seg = fetch16();
			strbuf.sprintf("%s[%s:%s]", str_sreg_ovr(sreg_ovr), str_imm(seg), str_imm(ofs));
		}
		break;
	case PARAM_REG8:
		{
			byte reg = (modrm >> 3) & 0b111;
			strbuf.sprintf("%s", str_reg(reg, false));
		}
		break;
	case PARAM_REG16:
		{
			byte reg = (modrm >> 3) & 0b111;
			strbuf.sprintf("%s", str_reg(reg, true));
		}
		break;
	case PARAM_REL8:
		{
			uint16_t inc = fetch8();
			strbuf.sprintf("%s", str_imm(ip + inc));
		}
		break;
	case PARAM_REL16:
		{
			uint16_t inc = fetch16();
			strbuf.sprintf("%s", str_imm(ip + inc));
		}
		break;

	case PARAM_MEM8:
	case PARAM_MEM16:
	case PARAM_MEM32:
	case PARAM_RM8:
	case PARAM_RM16:
		{
			byte mod = (modrm >> 6);
			byte rm  = (modrm >> 0) & 0b111;
			bool w   = (arg == PARAM_RM16)
					|| (arg == PARAM_MEM16);
					;
			bool ptr = (arg == PARAM_MEM8)
			        || (arg == PARAM_MEM16)
					|| (arg == PARAM_MEM32)
					;

			if (mod == 0b11) {
				if (arg == PARAM_MEM8 || arg == PARAM_MEM8 || arg == PARAM_MEM32) {
					strbuf.sprintf("invalid");
				} else {
					strbuf.sprintf("%s", str_reg(rm, w));
				}
			} else {
				strbuf.sprintf("%s", str_sreg_ovr(sreg_ovr));
				strbuf.sprintf("[");
				switch (rm) {
					case 0b000: strbuf.sprintf("bx+si"); break;
					case 0b001: strbuf.sprintf("bx+di"); break;
					case 0b010: strbuf.sprintf("bp+si"); break;
					case 0b011: strbuf.sprintf("bp+di"); break;
					case 0b100: strbuf.sprintf("si");    break;
					case 0b101: strbuf.sprintf("di");    break;
					case 0b110:
						if (mod) {
							strbuf.sprintf("bp");
						} else {
							strbuf.sprintf("%s", str_imm(fetch16()));
						}
						break;
					case 0b111: strbuf.sprintf("bx");    break;
				}
				int disp;
				switch (mod) {
					case 0b01:
						disp = fetch8();
						if (disp & 0x80) {
							strbuf.sprintf("-%s", str_imm((disp ^ 0xff) + 1));
						} else {
							strbuf.sprintf("+%s", str_imm(disp));
						}
						break;
					case 0b10:
						strbuf.sprintf("+%s", str_imm(fetch16()));
						break;
				}
				strbuf.sprintf("]");
			}
		}
		break;
	case PARAM_SI:
		strbuf.sprintf("si");
		break;
	case PARAM_SP:
		strbuf.sprintf("sp");
		break;
	case PARAM_SREG:
		{
			byte sreg = (modrm >> 3) & 0b111;
			strbuf.sprintf("%s", str_sreg(sreg));
		}
		break;
	case PARAM_SS:
		strbuf.sprintf("ss");
		break;
	}
}
