#include "disasm_i8086.h"

#include "emu/i8086.h"
#include "names.h"

enum {
	NONE,
	MODRM,
	GROUP,
};

enum {
	SEG_ES,
	SEG_CS,
	SEG_SS,
	SEG_DS,
};

/*
 * Checked against http://mlsite.net/8086/
 */

const opcode_t i8086_opcode_table[256] = {
	/* 00 */ { "add",    MODRM, PARAM_RM8,    PARAM_REG8,   RW, RO },
	/* 01 */ { "add",    MODRM, PARAM_RM16,   PARAM_REG16,  RW, RO },
	/* 02 */ { "add",    MODRM, PARAM_REG8,   PARAM_RM8,    RW, RO },
	/* 03 */ { "add",    MODRM, PARAM_REG16,  PARAM_RM16,   RW, RO },
	/* 04 */ { "add",    0,     PARAM_IMM8,   PARAM_NONE,   RO     },
	/* 05 */ { "add",    0,     PARAM_IMM16,  PARAM_NONE,   RO     },
	/* 06 */ { "push",   0,     PARAM_ES,     PARAM_NONE,   RO     },
	/* 07 */ { "pop",    0,     PARAM_ES,     PARAM_NONE,   RO     },
	/* 08 */ { "or",     MODRM, PARAM_RM8,    PARAM_REG8,   RW, RO },
	/* 09 */ { "or",     MODRM, PARAM_RM16,   PARAM_REG16,  RW, RO },
	/* 0a */ { "or",     MODRM, PARAM_REG8,   PARAM_RM8,    RW, RO },
	/* 0b */ { "or",     MODRM, PARAM_REG16,  PARAM_RM16,   RW, RO },
	/* 0c */ { "or",     0,     PARAM_IMM8,   PARAM_NONE,   RO     },
	/* 0d */ { "or",     0,     PARAM_IMM16,  PARAM_NONE,   RO     },
	/* 0e */ { "push",   0,     PARAM_CS,     PARAM_NONE,   RO     },
	/* 0f */ { "pop",    0,     PARAM_CS,     PARAM_NONE,   RO     },
	/* 10 */ { "adc",    MODRM, PARAM_RM8,    PARAM_REG8,   RW, RO },
	/* 11 */ { "adc",    MODRM, PARAM_RM16,   PARAM_REG16,  RW, RO },
	/* 12 */ { "adc",    MODRM, PARAM_REG8,   PARAM_RM8,    RW, RO },
	/* 13 */ { "adc",    MODRM, PARAM_REG16,  PARAM_RM16,   RW, RO },
	/* 14 */ { "adc",    0,     PARAM_IMM8,   PARAM_NONE,   RO     },
	/* 15 */ { "adc",    0,     PARAM_IMM16,  PARAM_NONE,   RO     },
	/* 16 */ { "push",   0,     PARAM_SS,     PARAM_NONE,   RO     },
	/* 17 */ { "pop",    0,     PARAM_SS,     PARAM_NONE,   RO     },
	/* 18 */ { "sbb",    MODRM, PARAM_RM8,    PARAM_REG8,   RW, RO },
	/* 19 */ { "sbb",    MODRM, PARAM_RM16,   PARAM_REG16,  RW, RO },
	/* 1a */ { "sbb",    MODRM, PARAM_REG8,   PARAM_RM8,    RW, RO },
	/* 1b */ { "sbb",    MODRM, PARAM_REG16,  PARAM_RM16,   RW, RO },
	/* 1c */ { "sbb",    0,     PARAM_IMM8,   PARAM_NONE,   RO     },
	/* 1d */ { "sbb",    0,     PARAM_IMM16,  PARAM_NONE,   RO     },
	/* 1e */ { "push",   0,     PARAM_DS,     PARAM_NONE,   RO     },
	/* 1f */ { "pop",    0,     PARAM_DS,     PARAM_NONE,   RO     },
	/* 20 */ { "and",    MODRM, PARAM_RM8,    PARAM_REG8,   RW, RO },
	/* 21 */ { "and",    MODRM, PARAM_RM16,   PARAM_REG16,  RW, RO },
	/* 22 */ { "and",    MODRM, PARAM_REG8,   PARAM_RM8,    RW, RO },
	/* 23 */ { "and",    MODRM, PARAM_REG16,  PARAM_RM16,   RW, RO },
	/* 24 */ { "and",    0,     PARAM_IMM8,   PARAM_NONE,   RO     },
	/* 25 */ { "and",    0,     PARAM_IMM16,  PARAM_NONE,   RO     },
	/* 26 */ {                                                     },
	/* 27 */ { "daa"                                               },
	/* 28 */ { "sub",    MODRM, PARAM_RM8,    PARAM_REG8,   RW, RO },
	/* 29 */ { "sub",    MODRM, PARAM_RM16,   PARAM_REG16,  RW, RO },
	/* 2a */ { "sub",    MODRM, PARAM_REG8,   PARAM_RM8,    RW, RO },
	/* 2b */ { "sub",    MODRM, PARAM_REG16,  PARAM_RM16,   RW, RO },
	/* 2c */ { "sub",    0,     PARAM_IMM8,   PARAM_NONE,   RO     },
	/* 2d */ { "sub",    0,     PARAM_IMM16,  PARAM_NONE,   RO     },
	/* 2e */ {                                                     },
	/* 2f */ { "das"                                               },
	/* 30 */ { "xor",    MODRM, PARAM_RM8,    PARAM_REG8,   RW, RO },
	/* 31 */ { "xor",    MODRM, PARAM_RM16,   PARAM_REG16,  RW, RO },
	/* 32 */ { "xor",    MODRM, PARAM_REG8,   PARAM_RM8,    RW, RO },
	/* 33 */ { "xor",    MODRM, PARAM_REG16,  PARAM_RM16,   RW, RO },
	/* 34 */ { "xor",    0,     PARAM_IMM8,   PARAM_NONE,   RO     },
	/* 35 */ { "xor",    0,     PARAM_IMM16,  PARAM_NONE,   RO     },
	/* 36 */ {                                                     },
	/* 37 */ { "aaa"                                               },
	/* 38 */ { "cmp",    MODRM, PARAM_RM8,    PARAM_REG8,   RO, RO },
	/* 39 */ { "cmp",    MODRM, PARAM_RM16,   PARAM_REG16,  RO, RO },
	/* 3a */ { "cmp",    MODRM, PARAM_REG8,   PARAM_RM8,    RO, RO },
	/* 3b */ { "cmp",    MODRM, PARAM_REG16,  PARAM_RM16,   RO, RO },
	/* 3c */ { "cmp",    0,     PARAM_IMM8,   PARAM_NONE,   RO     },
	/* 3d */ { "cmp",    0,     PARAM_IMM16,  PARAM_NONE,   RO     },
	/* 3e */ {                                                     },
	/* 3f */ { "aas"                                               },
	/* 40 */ { "inc",    0,     PARAM_AX,     PARAM_NONE,   RW     },
	/* 41 */ { "inc",    0,     PARAM_CX,     PARAM_NONE,   RW     },
	/* 42 */ { "inc",    0,     PARAM_DX,     PARAM_NONE,   RW     },
	/* 43 */ { "inc",    0,     PARAM_BX,     PARAM_NONE,   RW     },
	/* 44 */ { "inc",    0,     PARAM_SP,     PARAM_NONE,   RW     },
	/* 45 */ { "inc",    0,     PARAM_BP,     PARAM_NONE,   RW     },
	/* 46 */ { "inc",    0,     PARAM_SI,     PARAM_NONE,   RW     },
	/* 47 */ { "inc",    0,     PARAM_DI,     PARAM_NONE,   RW     },
	/* 48 */ { "dec",    0,     PARAM_AX,     PARAM_NONE,   RW     },
	/* 49 */ { "dec",    0,     PARAM_CX,     PARAM_NONE,   RW     },
	/* 4a */ { "dec",    0,     PARAM_DX,     PARAM_NONE,   RW     },
	/* 4b */ { "dec",    0,     PARAM_BX,     PARAM_NONE,   RW     },
	/* 4c */ { "dec",    0,     PARAM_SP,     PARAM_NONE,   RW     },
	/* 4d */ { "dec",    0,     PARAM_BP,     PARAM_NONE,   RW     },
	/* 4e */ { "dec",    0,     PARAM_SI,     PARAM_NONE,   RW     },
	/* 4f */ { "dec",    0,     PARAM_DI,     PARAM_NONE,   RW     },
	/* 50 */ { "push",   0,     PARAM_AX,     PARAM_NONE,   RO     },
	/* 51 */ { "push",   0,     PARAM_CX,     PARAM_NONE,   RO     },
	/* 52 */ { "push",   0,     PARAM_DX,     PARAM_NONE,   RO     },
	/* 53 */ { "push",   0,     PARAM_BX,     PARAM_NONE,   RO     },
	/* 54 */ { "push",   0,     PARAM_SP,     PARAM_NONE,   RO     },
	/* 55 */ { "push",   0,     PARAM_BP,     PARAM_NONE,   RO     },
	/* 56 */ { "push",   0,     PARAM_SI,     PARAM_NONE,   RO     },
	/* 57 */ { "push",   0,     PARAM_DI,     PARAM_NONE,   RO     },
	/* 58 */ { "pop",    0,     PARAM_AX,     PARAM_NONE,   WO     },
	/* 59 */ { "pop",    0,     PARAM_CX,     PARAM_NONE,   WO     },
	/* 5a */ { "pop",    0,     PARAM_DX,     PARAM_NONE,   WO     },
	/* 5b */ { "pop",    0,     PARAM_BX,     PARAM_NONE,   WO     },
	/* 5c */ { "pop",    0,     PARAM_SP,     PARAM_NONE,   WO     },
	/* 5d */ { "pop",    0,     PARAM_BP,     PARAM_NONE,   WO     },
	/* 5e */ { "pop",    0,     PARAM_SI,     PARAM_NONE,   WO     },
	/* 5f */ { "pop",    0,     PARAM_DI,     PARAM_NONE,   WO     },
	/* 60 */ {                                                     },
	/* 61 */ {                                                     },
	/* 62 */ {                                                     },
	/* 63 */ {                                                     },
	/* 64 */ {                                                     },
	/* 65 */ {                                                     },
	/* 66 */ {                                                     },
	/* 67 */ {                                                     },
	/* 68 */ {                                                     },
	/* 69 */ {                                                     },
	/* 6a */ {                                                     },
	/* 6b */ {                                                     },
	/* 6c */ {                                                     },
	/* 6d */ {                                                     },
	/* 6e */ {                                                     },
	/* 6f */ {                                                     },
	/* 70 */ { "jo",     0,     PARAM_REL8,   PARAM_NONE,   RO     },
	/* 71 */ { "jno",    0,     PARAM_REL8,   PARAM_NONE,   RO     },
	/* 72 */ { "jb",     0,     PARAM_REL8,   PARAM_NONE,   RO     },
	/* 73 */ { "jnb",    0,     PARAM_REL8,   PARAM_NONE,   RO     },
	/* 74 */ { "jz",     0,     PARAM_REL8,   PARAM_NONE,   RO     },
	/* 75 */ { "jnz",    0,     PARAM_REL8,   PARAM_NONE,   RO     },
	/* 76 */ { "jbe",    0,     PARAM_REL8,   PARAM_NONE,   RO     },
	/* 77 */ { "ja",     0,     PARAM_REL8,   PARAM_NONE,   RO     },
	/* 78 */ { "js",     0,     PARAM_REL8,   PARAM_NONE,   RO     },
	/* 79 */ { "jns",    0,     PARAM_REL8,   PARAM_NONE,   RO     },
	/* 7a */ { "jpe",    0,     PARAM_REL8,   PARAM_NONE,   RO     },
	/* 7b */ { "jpo",    0,     PARAM_REL8,   PARAM_NONE,   RO     },
	/* 7c */ { "jl",     0,     PARAM_REL8,   PARAM_NONE,   RO     },
	/* 7d */ { "jge",    0,     PARAM_REL8,   PARAM_NONE,   RO     },
	/* 7e */ { "jle",    0,     PARAM_REL8,   PARAM_NONE,   RO     },
	/* 7f */ { "jg",     0,     PARAM_REL8,   PARAM_NONE,   RO     },
	/* 80 */ { 0,        GROUP, PARAM_RM8,    PARAM_IMM8,   WO, RO }, // grp_1
	/* 81 */ { 0,        GROUP, PARAM_RM16,   PARAM_IMM16,  WO, RO }, // grp_1
	/* 82 */ { 0,        GROUP, PARAM_RM8,    PARAM_IMM8,   WO, RO }, // grp_1
	/* 83 */ { 0,        GROUP, PARAM_RM16,   PARAM_IMM8,   WO, RO }, // grp_1
	/* 84 */ { "test",   MODRM, PARAM_REG8,   PARAM_RM8,    RO, RO },
	/* 85 */ { "test",   MODRM, PARAM_REG16,  PARAM_RM16,   RO, RO },
	/* 86 */ { "xchg",   MODRM, PARAM_REG8,   PARAM_RM8,    RW, RW },
	/* 87 */ { "xchg",   MODRM, PARAM_REG16,  PARAM_RM16,   RW, RW },
	/* 88 */ { "mov",    MODRM, PARAM_RM8,    PARAM_REG8,   WO, RO },
	/* 89 */ { "mov",    MODRM, PARAM_RM16,   PARAM_REG16,  WO, RO },
	/* 8a */ { "mov",    MODRM, PARAM_REG8,   PARAM_RM8,    WO, RO },
	/* 8b */ { "mov",    MODRM, PARAM_REG16,  PARAM_RM16,   WO, RO },
	/* 8c */ { "mov",    MODRM, PARAM_RM16,   PARAM_SREG,   WO, RO },
	/* 8d */ { "lea",    MODRM, PARAM_REG16,  PARAM_MEM16,  RO, RO },
	/* 8e */ { "mov",    MODRM, PARAM_SREG,   PARAM_RM16,   WO, RO },
	/* 8f */ { "pop",    MODRM, PARAM_RM16,   PARAM_NONE,   WO     },
	/* 90 */ { "nop"                                               },
	/* 91 */ { "xchg",   0,     PARAM_CX,     PARAM_AX,     RW, RW },
	/* 92 */ { "xchg",   0,     PARAM_DX,     PARAM_AX,     RW, RW },
	/* 93 */ { "xchg",   0,     PARAM_BX,     PARAM_AX,     RW, RW },
	/* 94 */ { "xchg",   0,     PARAM_SP,     PARAM_AX,     RW, RW },
	/* 95 */ { "xchg",   0,     PARAM_BP,     PARAM_AX,     RW, RW },
	/* 96 */ { "xchg",   0,     PARAM_SI,     PARAM_AX,     RW, RW },
	/* 97 */ { "xchg",   0,     PARAM_DI,     PARAM_AX,     RW, RW },
	/* 98 */ { "cbw"                                               },
	/* 99 */ { "cwd"                                               },
	/* 9a */ { "call",   0,     PARAM_IMEM32, PARAM_NONE,   RO     },
	/* 9b */ { "wait"                                              },
	/* 9c */ { "pushf"                                             },
	/* 9d */ { "popf"                                              },
	/* 9e */ { "sahf"                                              },
	/* 9f */ { "lahf"                                              },
	/* a0 */ { "mov",    0,     PARAM_AL,     PARAM_IMEM8,  WO, RO },
	/* a1 */ { "mov",    0,     PARAM_AX,     PARAM_IMEM16, WO, RO },
	/* a2 */ { "mov",    0,     PARAM_IMEM8,  PARAM_AL,     WO, RO },
	/* a3 */ { "mov",    0,     PARAM_IMEM16, PARAM_AX,     WO, RO },
	/* a4 */ { "movsb"                                             },
	/* a5 */ { "movsw"                                             },
	/* a6 */ { "cmpsb"                                             },
	/* a7 */ { "cmpsw"                                             },
	/* a8 */ { "test",   0,     PARAM_AL,     PARAM_IMM8,   RO, RO },
	/* a9 */ { "test",   0,     PARAM_AX,     PARAM_IMM16,  RO, RO },
	/* aa */ { "stosb"                                             },
	/* ab */ { "stosw"                                             },
	/* ac */ { "lodsb"                                             },
	/* ad */ { "lodsw"                                             },
	/* ae */ { "scasb"                                             },
	/* af */ { "scasw"                                             },
	/* b0 */ { "mov",    0,     PARAM_AL,     PARAM_IMM8,   WO, RO },
	/* b1 */ { "mov",    0,     PARAM_CL,     PARAM_IMM8,   WO, RO },
	/* b2 */ { "mov",    0,     PARAM_DL,     PARAM_IMM8,   WO, RO },
	/* b3 */ { "mov",    0,     PARAM_BL,     PARAM_IMM8,   WO, RO },
	/* b4 */ { "mov",    0,     PARAM_AH,     PARAM_IMM8,   WO, RO },
	/* b5 */ { "mov",    0,     PARAM_CH,     PARAM_IMM8,   WO, RO },
	/* b6 */ { "mov",    0,     PARAM_DH,     PARAM_IMM8,   WO, RO },
	/* b7 */ { "mov",    0,     PARAM_BH,     PARAM_IMM8,   WO, RO },
	/* b8 */ { "mov",    0,     PARAM_AX,     PARAM_IMM16,  WO, RO },
	/* b9 */ { "mov",    0,     PARAM_CX,     PARAM_IMM16,  WO, RO },
	/* ba */ { "mov",    0,     PARAM_DX,     PARAM_IMM16,  WO, RO },
	/* bb */ { "mov",    0,     PARAM_BX,     PARAM_IMM16,  WO, RO },
	/* bc */ { "mov",    0,     PARAM_SP,     PARAM_IMM16,  WO, RO },
	/* bd */ { "mov",    0,     PARAM_BP,     PARAM_IMM16,  WO, RO },
	/* be */ { "mov",    0,     PARAM_SI,     PARAM_IMM16,  WO, RO },
	/* bf */ { "mov",    0,     PARAM_DI,     PARAM_IMM16,  WO, RO },
	/* c0 */ {                                                     },
	/* c1 */ {                                                     },
	/* c2 */ { "ret",    0,     PARAM_IMM16,  PARAM_NONE,   RO     },
	/* c3 */ { "ret"                                               },
	/* c4 */ { "les",    MODRM, PARAM_REG16,  PARAM_MEM32,  WO, RO },
	/* c5 */ { "lds",    MODRM, PARAM_REG16,  PARAM_MEM32,  WO, RO },
	/* c6 */ { "mov",    MODRM, PARAM_RM8,    PARAM_IMM8,   WO, RO },
	/* c7 */ { "mov",    MODRM, PARAM_RM16,   PARAM_IMM16,  WO, RO },
	/* c8 */ {                                                     },
	/* c9 */ {                                                     },
	/* ca */ { "retf",   0,     PARAM_IMM16,  PARAM_NONE,   RO     },
	/* cb */ { "retf"                                              },
	/* cc */ { "int",    0,     PARAM_3,      PARAM_NONE,   RO     },
	/* cd */ { "int",    0,     PARAM_IMM8,   PARAM_NONE,   RO     },
	/* ce */ { "into"                                              },
	/* cf */ { "iret"                                              },
	/* d0 */ { 0,        GROUP, PARAM_RM8,    PARAM_1,      RW, RO }, // grp_2
	/* d1 */ { 0,        GROUP, PARAM_RM16,   PARAM_1,      RW, RO }, // grp_2
	/* d2 */ { 0,        GROUP, PARAM_RM8,    PARAM_CL,     RW, RO }, // grp_2
	/* d3 */ { 0,        GROUP, PARAM_RM16,   PARAM_CL,     RW, RO }, // grp_2
	/* d4 */ { "aam",    0,     PARAM_IMM8,   PARAM_NONE,   RO     },
	/* d5 */ { "aad",    0,     PARAM_IMM8,   PARAM_NONE,   RO     },
	/* d6 */ {                                                     },
	/* d7 */ { "xlat"                                              },
	/* d8 */ {                                                     },
	/* d9 */ {                                                     },
	/* da */ {                                                     },
	/* db */ {                                                     },
	/* dc */ {                                                     },
	/* dd */ {                                                     },
	/* de */ {                                                     },
	/* df */ {                                                     },
	/* e0 */ { "loopnz", 0,     PARAM_REL8,   PARAM_NONE,   RO     },
	/* e1 */ { "loopz",  0,     PARAM_REL8,   PARAM_NONE,   RO     },
	/* e2 */ { "loop",   0,     PARAM_REL8,   PARAM_NONE,   RO     },
	/* e3 */ { "jcxz",   0,     PARAM_REL8,   PARAM_NONE,   RO     },
	/* e4 */ { "in",     0,     PARAM_AL,     PARAM_IMM8,   WO, RO },
	/* e5 */ { "in",     0,     PARAM_AX,     PARAM_IMM8,   WO, RO },
	/* e6 */ { "out",    0,     PARAM_IMM8,   PARAM_AL,     RO, RO },
	/* e7 */ { "out",    0,     PARAM_IMM8,   PARAM_AX,     RO, RO },
	/* e8 */ { "call",   0,     PARAM_REL16,  PARAM_NONE,   RO     },
	/* e9 */ { "jmp",    0,     PARAM_REL16,  PARAM_NONE,   RO     },
	/* ea */ { "jmp",    0,     PARAM_IMEM32, PARAM_NONE,   RO     },
	/* eb */ { "jmp",    0,     PARAM_REL8,   PARAM_NONE,   RO     },
	/* ec */ { "in",     0,     PARAM_AL,     PARAM_DX,     WO, RO },
	/* ed */ { "in",     0,     PARAM_AX,     PARAM_DX,     WO, RO },
	/* ee */ { "out",    0,     PARAM_DX,     PARAM_AL,     RO, RO },
	/* ef */ { "out",    0,     PARAM_DX,     PARAM_AX,     RO, RO },
	/* f0 */ { "lock"                                              },
	/* f1 */ {                                                     },
	/* f2 */ {                                                     },
	/* f3 */ {                                                     },
	/* f4 */ { "hlt"                                               },
	/* f5 */ { "cmc"                                               },
	/* f6 */ { 0,        GROUP, PARAM_RM8,    PARAM_IMM8,   RW, RO }, // grp_3
	/* f7 */ { 0,        GROUP, PARAM_RM16,   PARAM_IMM16,  RW, RO }, // grp_3
	/* f8 */ { "clc"                                               },
	/* f9 */ { "stc"                                               },
	/* fa */ { "cli"                                               },
	/* fb */ { "sti"                                               },
	/* fc */ { "cld"                                               },
	/* fd */ { "std"                                               },
	/* fe */ { 0,        GROUP, PARAM_RM8,    PARAM_NONE,   RW     }, // grp_4
	/* ff */ { 0,        GROUP                                     }, // grp_5
};

const opcode_t i8086_opcode_table_grp_1[8] = {
	{ "add" }, { "or"  }, { "adc" }, { "sbb" }, { "and" }, { "sub" }, { "xor" }, { "cmp" },
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
	{ "inc",  0, PARAM_RM16,  PARAM_NONE, WO },
	{ "dec",  0, PARAM_RM16,  PARAM_NONE, WO },
	{ "call", 0, PARAM_RM16,  PARAM_NONE, RO },
	{ "call", 0, PARAM_MEM32, PARAM_NONE, RO },
	{ "jmp",  0, PARAM_RM16,  PARAM_NONE, RO },
	{ "jmp",  0, PARAM_MEM32, PARAM_NONE, RO },
	{ "push", 0, PARAM_RM16,  PARAM_NONE, RO },
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
	byte v = read8(m_cs, m_ip + m_len);
	m_len += 1;
	return v;
}

uint16_t disasm_i8086_t::fetch16() {
	uint16_t v = read16(m_cs, m_ip + m_len);
	m_ip += 2;
	return v;
}

uint32_t disasm_i8086_t::fetch32() {
	uint16_t vl = read16(m_cs, m_ip + m_len + 0);
	uint16_t vh = read16(m_cs, m_ip + m_len + 2);
	uint32_t v = (vh << 16) + vl;
	m_len += 4;
	return v;
}

static bool needs_modrm(const opcode_t &opcode) {
	return opcode.flags == MODRM || opcode.flags == GROUP;
}

static opcode_t inherit(const opcode_t op_detail, const opcode_t grp) {
	return opcode_t {
		.mnemonic = op_detail.mnemonic,
		.flags = op_detail.flags,
		.arg_1 = op_detail.arg_1 == PARAM_INHERIT ? grp.arg_1 : op_detail.arg_1,
		.arg_2 = op_detail.arg_2 == PARAM_INHERIT ? grp.arg_2 : op_detail.arg_2,
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

static int needs_width_for_mem(arg_type_e arg) {
	switch (arg) {
		case PARAM_INHERIT:
		case PARAM_NONE:
		case PARAM_IMM8:
		case PARAM_IMM16:
			return true;

		default:;
	}
	return false;
}

void disasm_i8086_t::set_cpu(i8086_t *cpu) {
	m_cpu = cpu;
}

void disasm_i8086_t::set_names(const names_t *a_names) {
	names = a_names;
}

void disasm_i8086_t::decode(uint16_t cs, uint16_t ip) {
	m_cs = cs;
	m_ip = ip;
	m_len = 0;

	m_sreg_ovr  = 0;
	m_flag_f2   = false;
	m_flag_f3   = false;
	m_flag_lock = false;
	m_op        = 0;
	m_modrm     = 0;

	byte op;
	for (;;) {
		op = fetch8();

		// Handle prefixes
		switch (op) {
			case 0x2e: m_sreg_ovr = op; break; // cs:
			case 0x36: m_sreg_ovr = op; break; // ss:
			case 0x3e: m_sreg_ovr = op; break; // ds:
			case 0x26: m_sreg_ovr = op; break; // es:
			case 0xf0: // lock
				m_flag_lock = true;
				break;
			case 0xf2: // repne
				m_flag_f2 = true;
				m_flag_f3 = false;
				break;
			case 0xf3: // rep/repe
				m_flag_f3 = true;
				m_flag_f2 = false;
				break;
			default:
				goto opcode;
		}
	}
opcode:

	opcode_t opcode = i8086_opcode_table[op];

	if (needs_modrm(opcode)) {
		m_modrm = fetch8();
	}

	byte reg = ((m_modrm >> 3) & 0b111);

	switch (op) {
		case 0x80: case 0x81: case 0x82: case 0x83:
			opcode = inherit(i8086_opcode_table_grp_1[reg], opcode); break;
		case 0xd0: case 0xd1: case 0xd2: case 0xd3:
			opcode = inherit(i8086_opcode_table_grp_2[reg], opcode); break;
		case 0xf6: case 0xf7:
			opcode = inherit(i8086_opcode_table_grp_3[reg], opcode); break;
		case 0xfe:
			opcode = inherit(i8086_opcode_table_grp_4[reg], opcode); break;
		case 0xff:
			opcode = inherit(i8086_opcode_table_grp_5[reg], opcode); break;
	}

	if (opcode.arg_1 == PARAM_INHERIT) {
		opcode.arg_1 = PARAM_NONE;
	}
	if (opcode.arg_2 == PARAM_INHERIT) {
		opcode.arg_2 = PARAM_NONE;
	}

	if (opcode.arg_1) {
		decode_param(0, opcode.arg_1);
	}
	if (opcode.arg_2) {
		decode_param(1, opcode.arg_2);
	}

	m_opcode = opcode;

	m_has_mem_arg = is_mem_arg(m_opcode.arg_1, m_modrm)
				 || is_mem_arg(m_opcode.arg_2, m_modrm);
}

void disasm_i8086_t::disassemble(uint16_t a_cs, uint16_t *a_ip, const char **s) {
	decode(a_cs, *a_ip);

	strbuf.clear();

	if (!m_opcode.mnemonic) {
		strbuf.sprintf("db\t%s", str_imm(m_op));
	} else {
		if (m_flag_lock) {
			strbuf.sprintf("lock ");
		}
		if (m_flag_f2) {
			switch (m_op) {
				case 0xa6: // cmpsb
				case 0xa7: // cmpsw
					strbuf.sprintf("repne ");
					break;
				case 0xae: // scasb
				case 0xaf: // scasw
					strbuf.sprintf("repne ");
					break;
			}
		} else if (m_flag_f3) {
			switch (m_op) {
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

		/*
		 * If we have a segment override but no memory
		 * argument to put it in front of, put it before
		 * the mnemonic.
		 */
		if (m_sreg_ovr) {
			if (!m_has_mem_arg) {
				strbuf.sprintf("%s", str_sreg_ovr(m_sreg_ovr));
			}
		}

		int col = strbuf.get_len();
		strbuf.sprintf("%s", m_opcode.mnemonic);

		bool needs_mem_width = m_has_mem_arg &&
			(needs_width_for_mem(m_opcode.arg_1) ||
			 needs_width_for_mem(m_opcode.arg_2));

		bool show_mem_width = always_show_mem_width || needs_mem_width;

		if (m_opcode.arg_1 != PARAM_NONE) {
			strbuf.align_col(col + 8);
			str_param(0, m_opcode.arg_1, show_mem_width);
		}
		if (m_opcode.arg_2 != PARAM_NONE) {
			strbuf.sprintf(", ");
			str_param(1, m_opcode.arg_2, show_mem_width);
		}

		if (m_has_mem_arg && m_cpu) {
			strbuf.align_col(col + 28);
			std::optional<mem_ref_t> mem_ref = get_mem_arg();
			if (mem_ref.has_value() && mem_ref->width) {
				strbuf.sprintf("\t[%s:", str_imm(mem_ref->seg));
				strbuf.sprintf("%s] = ", str_imm(mem_ref->ofs));
				strbuf.sprintf("%s", str_imm(mem_ref->value));
			}
		}
	}

	*a_ip = m_ip + m_len;
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
		case 0x26: return "es:";
		case 0x2e: return "cs:";
		case 0x36: return "ss:";
		case 0x3e: return "ds:";
	}
	return "";
}

void disasm_i8086_t::decode_param(int n, arg_type_e arg) {
	switch (arg) {
		case PARAM_IMM8:
		case PARAM_REL8:
			m_imm[n] = fetch8();
			break;
		case PARAM_IMEM8:
		case PARAM_IMEM16:
		case PARAM_IMM16:
		case PARAM_REL16:
			m_imm[n] = fetch16();
			break;
		case PARAM_IMEM32:
			m_imm[n] = fetch32();
			break;
		case PARAM_MEM8:
		case PARAM_MEM16:
		case PARAM_MEM32:
		case PARAM_RM8:
		case PARAM_RM16:
			{
				byte mod = (m_modrm >> 6);
				byte rm  = (m_modrm >> 0) & 0b111;

				switch (mod) {
					case 0b00:
						if (rm == 0b110) {
							m_imm[n] = fetch16();
						}
						break;
					case 0b01:
						m_imm[n] = fetch8();
						break;
					case 0b10:
						m_imm[n] = fetch16();
						break;
					case 0b11:
						break;
				}
			}
			break;
		default:
			break;
	}
}

void disasm_i8086_t::str_param(int n, arg_type_e arg, bool show_mem_width) {
	switch (arg) {
		case PARAM_INHERIT:
		case PARAM_NONE:
			break;

		case PARAM_1:  strbuf.sprintf("1"); break;
		case PARAM_3:  strbuf.sprintf("3"); break;

		case PARAM_AL: strbuf.sprintf("al"); break;
		case PARAM_CL: strbuf.sprintf("cl"); break;
		case PARAM_DL: strbuf.sprintf("dl"); break;
		case PARAM_BL: strbuf.sprintf("bl"); break;

		case PARAM_AH: strbuf.sprintf("ah"); break;
		case PARAM_CH: strbuf.sprintf("ch"); break;
		case PARAM_DH: strbuf.sprintf("dh"); break;
		case PARAM_BH: strbuf.sprintf("bh"); break;

		case PARAM_AX: strbuf.sprintf("ax"); break;
		case PARAM_CX: strbuf.sprintf("cx"); break;
		case PARAM_DX: strbuf.sprintf("dx"); break;
		case PARAM_BX: strbuf.sprintf("bx"); break;

		case PARAM_SP: strbuf.sprintf("sp"); break;
		case PARAM_BP: strbuf.sprintf("bp"); break;
		case PARAM_SI: strbuf.sprintf("si"); break;
		case PARAM_DI: strbuf.sprintf("di"); break;

		case PARAM_ES: strbuf.sprintf("es"); break;
		case PARAM_CS: strbuf.sprintf("cs"); break;
		case PARAM_SS: strbuf.sprintf("ss"); break;
		case PARAM_DS: strbuf.sprintf("ds"); break;

		case PARAM_REG8:
		case PARAM_REG16:
			{
				byte reg = ((m_modrm >> 3) & 0b111);
				strbuf.sprintf("%s", str_reg(reg, arg == PARAM_REG16));
			}
			break;

		case PARAM_SREG:
			{
				byte reg = ((m_modrm >> 3) & 0b111);
				strbuf.sprintf("%s", str_sreg(reg));
			}
			break;

		case PARAM_IMM8:
		case PARAM_IMM16:
			strbuf.sprintf("%s", str_imm(m_imm[n]));
			break;

		case PARAM_REL8:
		case PARAM_REL16:
			{
				uint16_t inc = m_imm[n];
				strbuf.sprintf("%s", str_imm(m_ip + inc));
			}
			break;

		case PARAM_IMEM8:
		case PARAM_IMEM16:
			strbuf.sprintf("%s[%s]", str_sreg_ovr(m_sreg_ovr), str_imm(m_imm[n]));
			break;

		case PARAM_IMEM32:
			{
				uint16_t ofs = m_imm[n] & 0xffff;
				uint16_t seg = m_imm[n] > 16;
				strbuf.sprintf("%s[%s:%s]", str_sreg_ovr(m_sreg_ovr), str_imm(seg), str_imm(ofs));
				m_imm[n] = (ofs << 16) + seg;
			}
			break;

		case PARAM_MEM8:
		case PARAM_MEM16:
		case PARAM_MEM32:
		case PARAM_RM8:
		case PARAM_RM16:
			{
				byte mod = (m_modrm >> 6);
				byte rm  = (m_modrm >> 0) & 0b111;

				bool w = (arg == PARAM_RM16)
					  || (arg == PARAM_MEM16);

				if (mod == 0b11) {
					if (arg == PARAM_MEM8 || arg == PARAM_MEM8 || arg == PARAM_MEM32) {
						strbuf.sprintf("invalid");
					} else {
						strbuf.sprintf("%s", str_reg(rm, w));
					}
				} else {
					if (show_mem_width) {
						switch (arg) {
							case PARAM_MEM8:
							case PARAM_RM8:
								strbuf.sprintf("byte ptr ");
								break;
							case PARAM_RM16:
							case PARAM_MEM16:
								strbuf.sprintf("word ptr ");
								break;
							case PARAM_MEM32:
								strbuf.sprintf("far ptr ");
								break;
							default:;
						}
					}

					strbuf.sprintf("%s", str_sreg_ovr(m_sreg_ovr));
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
								strbuf.sprintf("%s", str_imm(m_imm[n]));
							}
							break;
						case 0b111: strbuf.sprintf("bx");    break;
					}
					switch (mod) {
						case 0b01:
							if (m_imm[n] & 0x80) {
								strbuf.sprintf("-%s", str_imm((m_imm[n] ^ 0xff) + 1));
							} else {
								strbuf.sprintf("+%s", str_imm(m_imm[n]));
							}
							break;
						case 0b10:
							strbuf.sprintf("+%s", str_imm(m_imm[n]));
							break;
					}
					strbuf.sprintf("]");
				}
			}
			break;
	}
}

uint16_t disasm_i8086_t::read_sreg(byte sreg) {
	switch (sreg) {
		case SEG_ES: return m_cpu->es; break;
		case SEG_CS: return m_cpu->cs; break;
		case SEG_SS: return m_cpu->ss; break;
		case SEG_DS: return m_cpu->ds; break;
		default: break;
	}
	assert(0 && "invalid sreg");
	return 0;
}

byte disasm_i8086_t::get_sreg_ovr(byte sreg_def) {
	byte sreg = sreg_def;
	if (m_sreg_ovr) {
		sreg = (m_sreg_ovr >> 3) & 0b11;
	}
	return sreg;
}

uint16_t disasm_i8086_t::read_sreg_ovr(byte sreg_def) {
	byte sreg = get_sreg_ovr(sreg_def);
	return read_sreg(sreg);
}

static inline
uint16_t sext(byte v) {
	if (v & 0x80) {
		return 0xff00 | uint16_t(v);
	}
	return v;
}

std::optional<mem_ref_t> disasm_i8086_t::get_mem_arg() {
	auto evaluate_arg = [&](int n, arg_type_e arg) -> mem_ref_t {
		uint16_t seg = 0, ofs = 0;
		uint32_t v = 0;
		byte w = 0;
		switch (arg) {
			case PARAM_IMEM8:
				seg = read_sreg_ovr(SEG_DS);
				ofs = m_imm[n];
				v = read8(seg, ofs);
				w = 1;
				break;
			case PARAM_IMEM16:
				seg = read_sreg_ovr(SEG_DS);
				ofs = m_imm[n];
				v = read16(seg, ofs);
				w = 2;
				break;
			case PARAM_IMEM32:
				ofs = m_imm[n] >> 16;
				seg = m_imm[n] & 0xffff;
				v = read16(seg, ofs);
				w = 2;
				break;
			case PARAM_MEM8:
			case PARAM_MEM16:
			case PARAM_MEM32:
			case PARAM_RM8:
			case PARAM_RM16:
			{
				byte mod = (m_modrm >> 6);
				byte rm  = (m_modrm >> 0) & 0b111;

				seg = read_sreg_ovr(SEG_DS);

				ofs = 0;
				switch (rm) {
					case 0b000: ofs = m_cpu->bx + m_cpu->si; break;
					case 0b001: ofs = m_cpu->bx + m_cpu->di; break;
					case 0b010: ofs = m_cpu->bp + m_cpu->si; break;
					case 0b011: ofs = m_cpu->bp + m_cpu->di; break;
					case 0b100: ofs = m_cpu->si;             break;
					case 0b101: ofs = m_cpu->di;             break;
					case 0b110:
						if (mod) {
							ofs = m_cpu->bp;
						} else {
							ofs = m_imm[n];
						}
						break;
					case 0b111: ofs = m_cpu->bx; break;
				}
				switch (mod) {
					case 0b01:
						ofs += sext(m_imm[n]);
						break;
					case 0b10:
						ofs += m_imm[n];
						break;
				}

				w = (arg == PARAM_RM16) || (arg == PARAM_MEM16);

				v = !w ? read8(seg, ofs) : read16(seg, ofs);
			}
			break;
			default:
				break;
		}
		return mem_ref_t { seg, ofs, v, w };
	};

	auto reads_from_arg = [](arg_dir_e dir) -> bool {
		return dir == RO || dir == RW;
	};

	if (is_mem_arg(m_opcode.arg_1, m_modrm) && reads_from_arg(m_opcode.arg_1_dir)) {
		return evaluate_arg(0, m_opcode.arg_1);
	} else if (is_mem_arg(m_opcode.arg_2, m_modrm) && reads_from_arg(m_opcode.arg_2_dir)) {
		return evaluate_arg(1, m_opcode.arg_2);
	}
	return {};
}
