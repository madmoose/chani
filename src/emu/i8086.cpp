#include "i8086.h"

#include <cassert>
#include <cstdio>
#include <cctype>

#include "support/endian.h"
#include "ibm5160.h"

#define CHANIDEBUG 0
#define CPULOG     0

i8086_t::i8086_t() {
	op = 0;
	is_prefix = false;
	sreg_ovr = 0;
	repmode = REP_NONE;
	memory = nullptr;
}

void i8086_t::set_machine(ibm5160_t *a_machine) {
	machine = a_machine;
}

void i8086_t::set_memory(byte *a_memory) {
	memory = a_memory;
}

void i8086_t::dump_state() {
	printf("\n\t");
	printf("ax=%04x\t", ax);
	printf("es=%04x\t", es);
	printf("sp=%04x\t", sp);
	printf("ip=%04x\n", ip);

	printf("\t");
	printf("cx=%04x\t", cx);
	printf("cs=%04x\t", cs);
	printf("bp=%04x\t", bp);
	printf("op=%02x\n", op);

	printf("\t");
	printf("dx=%04x\t", dx);
	printf("ss=%04x\t", ss);
	printf("di=%04x\n", di);

	printf("\t");
	printf("bx=%04x\t", bx);
	printf("ds=%04x\t", ds);
	printf("si=%04x\t", si);

	printf("O%d ", flags.of);
	printf("D%d ", flags.df);
	printf("I%d ", flags.if_);
	printf("T%d ", flags.tf);
	printf("S%d ", flags.sf);
	printf("Z%d ", flags.zf);
	printf("A%d ", flags.af);
	printf("P%d ", flags.pf);
	printf("C%d\n\n", flags.cf);
}

void i8086_t::log_state() {
	uint16_t psw = 0;

	psw |= flags.of  ? (1 << 11) : 0;
	psw |= flags.df  ? (1 << 10) : 0;
	psw |= flags.if_ ? (1 <<  9) : 0;
	psw |= flags.tf  ? (1 <<  8) : 0;
	psw |= flags.sf  ? (1 <<  7) : 0;
	psw |= flags.zf  ? (1 <<  6) : 0;
	// psw |= flags.af  ? (1 <<  4) : 0;
	psw |= flags.pf  ? (1 <<  2) : 0;
	psw |= flags.cf  ? (1 <<  0) : 0;

	printf("%10d: ", instr_count);
	printf("%04X:%04X  ", cs, ip);
	printf(" EAX:%08X", ax);
	printf(" EBX:%08X", bx);
	printf(" ECX:%08X", cx);
	printf(" EDX:%08X", dx);
	printf(" ESI:%08X", si);
	printf(" EDI:%08X", di);
	printf(" EBP:%08X", bp);
	printf(" ESP:%08X", sp);
	printf(" DS:%04X", ds);
	printf(" ES:%04X", es);
	printf(" SS:%04X", ss);
	printf(" C%d", flags.cf);
	printf(" Z%d", flags.zf);
	printf(" S%d", flags.sf);
	printf(" O%d", flags.of);
	printf(" I%d", flags.if_);
	printf(" F%04X", psw);
	printf("\n");
}

uint32_t i8086_t::step() {
	op_ip = ip;

	sreg_ovr = 0;
	repmode = REP_NONE;
	uint32_t cycles = 0;

	if (CHANIDEBUG) {
		dump_state();
		printf("%04x:%04x (%d)\t", cs, ip, instr_count);
	}

	if (CPULOG) {
		log_state();
	}

	instr_count++;

	if (instr_count % 1000000 == 0) {
		printf("inst_count: %d\n", instr_count / 1000000);
	}

	if (instr_count > 16000000) {
		exit(0);
	}

	// if (ip == 0xf314) {
	// 	char b[16];
	// 	uint16_t si_ = si;
	// 	int len = 0;
	// 	while (len < 15) {
	// 		char c = read_mem8(ds, si + len);
	// 		b[len++] = c;
	// 		if (!c) {
	// 			break;
	// 		}
	// 	}
	// 	printf("Opening resource '%s'\n", b);
	// }

	do {
		op = fetch8();
		is_prefix = false;
		cycles += dispatch();
	} while (is_prefix);

	return cycles;
}

uint32_t i8086_t::dispatch() {
#define OPCODE(x, func) case x: op_##func(); break

	switch (op) {
		OPCODE(0x00, alu_r_rm);
		OPCODE(0x01, alu_r_rm);
		OPCODE(0x02, alu_r_rm);
		OPCODE(0x03, alu_r_rm);
		OPCODE(0x04, alu_a_imm);
		OPCODE(0x05, alu_a_imm);
		OPCODE(0x06, push_sreg);
		OPCODE(0x07, pop_sreg);
		OPCODE(0x08, alu_r_rm);
		OPCODE(0x09, alu_r_rm);
		OPCODE(0x0a, alu_r_rm);
		OPCODE(0x0b, alu_r_rm);
		OPCODE(0x0c, alu_a_imm);
		OPCODE(0x0d, alu_a_imm);
		OPCODE(0x0e, push_sreg);
		OPCODE(0x0f, unused);
		OPCODE(0x10, alu_r_rm);
		OPCODE(0x11, alu_r_rm);
		OPCODE(0x12, alu_r_rm);
		OPCODE(0x13, alu_r_rm);
		OPCODE(0x14, alu_a_imm);
		OPCODE(0x15, alu_a_imm);
		OPCODE(0x16, push_sreg);
		OPCODE(0x17, pop_sreg);
		OPCODE(0x18, alu_r_rm);
		OPCODE(0x19, alu_r_rm);
		OPCODE(0x1a, alu_r_rm);
		OPCODE(0x1b, alu_r_rm);
		OPCODE(0x1c, alu_a_imm);
		OPCODE(0x1d, alu_a_imm);
		OPCODE(0x1e, push_sreg);
		OPCODE(0x1f, pop_sreg);
		OPCODE(0x20, alu_r_rm);
		OPCODE(0x21, alu_r_rm);
		OPCODE(0x22, alu_r_rm);
		OPCODE(0x23, alu_r_rm);
		OPCODE(0x24, alu_a_imm);
		OPCODE(0x25, alu_a_imm);
		OPCODE(0x26, seg_ovr_es);
		OPCODE(0x27, daa);
		OPCODE(0x28, alu_r_rm);
		OPCODE(0x29, alu_r_rm);
		OPCODE(0x2a, alu_r_rm);
		OPCODE(0x2b, alu_r_rm);
		OPCODE(0x2c, alu_a_imm);
		OPCODE(0x2d, alu_a_imm);
		OPCODE(0x2e, seg_ovr_cs);
		OPCODE(0x2f, das);
		OPCODE(0x30, alu_r_rm);
		OPCODE(0x31, alu_r_rm);
		OPCODE(0x32, alu_r_rm);
		OPCODE(0x33, alu_r_rm);
		OPCODE(0x34, alu_a_imm);
		OPCODE(0x35, alu_a_imm);
		OPCODE(0x36, seg_ovr_ss);
		OPCODE(0x37, aaa);
		OPCODE(0x38, alu_r_rm);
		OPCODE(0x39, alu_r_rm);
		OPCODE(0x3a, alu_r_rm);
		OPCODE(0x3b, alu_r_rm);
		OPCODE(0x3c, alu_a_imm);
		OPCODE(0x3d, alu_a_imm);
		OPCODE(0x3e, seg_ovr_ds);
		OPCODE(0x3f, aas);
		OPCODE(0x40, inc_reg);
		OPCODE(0x41, inc_reg);
		OPCODE(0x42, inc_reg);
		OPCODE(0x43, inc_reg);
		OPCODE(0x44, inc_reg);
		OPCODE(0x45, inc_reg);
		OPCODE(0x46, inc_reg);
		OPCODE(0x47, inc_reg);
		OPCODE(0x48, dec_reg);
		OPCODE(0x49, dec_reg);
		OPCODE(0x4a, dec_reg);
		OPCODE(0x4b, dec_reg);
		OPCODE(0x4c, dec_reg);
		OPCODE(0x4d, dec_reg);
		OPCODE(0x4e, dec_reg);
		OPCODE(0x4f, dec_reg);
		OPCODE(0x50, push_reg);
		OPCODE(0x51, push_reg);
		OPCODE(0x52, push_reg);
		OPCODE(0x53, push_reg);
		OPCODE(0x54, push_reg);
		OPCODE(0x55, push_reg);
		OPCODE(0x56, push_reg);
		OPCODE(0x57, push_reg);
		OPCODE(0x58, pop_reg);
		OPCODE(0x59, pop_reg);
		OPCODE(0x5a, pop_reg);
		OPCODE(0x5b, pop_reg);
		OPCODE(0x5c, pop_reg);
		OPCODE(0x5d, pop_reg);
		OPCODE(0x5e, pop_reg);
		OPCODE(0x5f, pop_reg);
		OPCODE(0x60, unused);
		OPCODE(0x61, unused);
		OPCODE(0x62, unused);
		OPCODE(0x63, unused);
		OPCODE(0x64, unused);
		OPCODE(0x65, unused);
		OPCODE(0x66, unused);
		OPCODE(0x67, unused);
		OPCODE(0x68, unused);
		OPCODE(0x69, unused);
		OPCODE(0x6a, unused);
		OPCODE(0x6b, unused);
		OPCODE(0x6c, unused);
		OPCODE(0x6d, unused);
		OPCODE(0x6e, unused);
		OPCODE(0x6f, unused);
		OPCODE(0x70, jcc);
		OPCODE(0x71, jcc);
		OPCODE(0x72, jcc);
		OPCODE(0x73, jcc);
		OPCODE(0x74, jcc);
		OPCODE(0x75, jcc);
		OPCODE(0x76, jcc);
		OPCODE(0x77, jcc);
		OPCODE(0x78, jcc);
		OPCODE(0x79, jcc);
		OPCODE(0x7a, jcc);
		OPCODE(0x7b, jcc);
		OPCODE(0x7c, jcc);
		OPCODE(0x7d, jcc);
		OPCODE(0x7e, jcc);
		OPCODE(0x7f, jcc);
		OPCODE(0x80, grp1_rmw_imm);
		OPCODE(0x81, grp1_rmw_imm);
		OPCODE(0x82, grp1_rmw_imm);
		OPCODE(0x83, grp1_rmw_imm);
		OPCODE(0x84, test_rm8_r8);
		OPCODE(0x85, test_rm16_r16);
		OPCODE(0x86, xchg_rm_r);
		OPCODE(0x87, xchg_rm_r);
		OPCODE(0x88, mov_rm_r);
		OPCODE(0x89, mov_rm_r);
		OPCODE(0x8a, mov_rm_r);
		OPCODE(0x8b, mov_rm_r);
		OPCODE(0x8c, mov_rm16_sreg);
		OPCODE(0x8d, lea_r16_m16);
		OPCODE(0x8e, mov_rm16_sreg);
		OPCODE(0x8f, pop_rm16);
		OPCODE(0x90, xchg_ax_r);
		OPCODE(0x91, xchg_ax_r);
		OPCODE(0x92, xchg_ax_r);
		OPCODE(0x93, xchg_ax_r);
		OPCODE(0x94, xchg_ax_r);
		OPCODE(0x95, xchg_ax_r);
		OPCODE(0x96, xchg_ax_r);
		OPCODE(0x97, xchg_ax_r);
		OPCODE(0x98, cbw);
		OPCODE(0x99, cwd);
		OPCODE(0x9a, call_far);
		OPCODE(0x9b, wait);
		OPCODE(0x9c, pushf);
		OPCODE(0x9d, popf);
		OPCODE(0x9e, sahf);
		OPCODE(0x9f, lahf);
		OPCODE(0xa0, mov_a_m);
		OPCODE(0xa1, mov_a_m);
		OPCODE(0xa2, mov_a_m);
		OPCODE(0xa3, mov_a_m);
		OPCODE(0xa4, movs);
		OPCODE(0xa5, movs);
		OPCODE(0xa6, cmps);
		OPCODE(0xa7, cmps);
		OPCODE(0xa8, test_a_imm);
		OPCODE(0xa9, test_a_imm);
		OPCODE(0xaa, stos);
		OPCODE(0xab, stos);
		OPCODE(0xac, lods);
		OPCODE(0xad, lods);
		OPCODE(0xae, scas);
		OPCODE(0xaf, scas);
		OPCODE(0xb0, mov_reg_imm);
		OPCODE(0xb1, mov_reg_imm);
		OPCODE(0xb2, mov_reg_imm);
		OPCODE(0xb3, mov_reg_imm);
		OPCODE(0xb4, mov_reg_imm);
		OPCODE(0xb5, mov_reg_imm);
		OPCODE(0xb6, mov_reg_imm);
		OPCODE(0xb7, mov_reg_imm);
		OPCODE(0xb8, mov_reg_imm);
		OPCODE(0xb9, mov_reg_imm);
		OPCODE(0xba, mov_reg_imm);
		OPCODE(0xbb, mov_reg_imm);
		OPCODE(0xbc, mov_reg_imm);
		OPCODE(0xbd, mov_reg_imm);
		OPCODE(0xbe, mov_reg_imm);
		OPCODE(0xbf, mov_reg_imm);
		OPCODE(0xc0, unused);
		OPCODE(0xc1, unused);
		OPCODE(0xc2, ret_imm16_intraseg);
		OPCODE(0xc3, ret_intraseg);
		OPCODE(0xc4, les_r16_m16);
		OPCODE(0xc5, lds_r16_m16);
		OPCODE(0xc6, mov_m_imm);
		OPCODE(0xc7, mov_m_imm);
		OPCODE(0xc8, unused);
		OPCODE(0xc9, unused);
		OPCODE(0xca, ret_imm16_interseg);
		OPCODE(0xcb, ret_interseg);
		OPCODE(0xcc, int_3);
		OPCODE(0xcd, int_imm8);
		OPCODE(0xce, into);
		OPCODE(0xcf, iret);
		OPCODE(0xd0, grp2_rmw);
		OPCODE(0xd1, grp2_rmw);
		OPCODE(0xd2, grp2_rmw);
		OPCODE(0xd3, grp2_rmw);
		OPCODE(0xd4, aam);
		OPCODE(0xd5, aad);
		OPCODE(0xd6, unused);
		OPCODE(0xd7, xlat);
		OPCODE(0xd8, esc);
		OPCODE(0xd9, esc);
		OPCODE(0xda, esc);
		OPCODE(0xdb, esc);
		OPCODE(0xdc, esc);
		OPCODE(0xdd, esc);
		OPCODE(0xde, esc);
		OPCODE(0xdf, esc);
		OPCODE(0xe0, loopnz);
		OPCODE(0xe1, loopz);
		OPCODE(0xe2, loop);
		OPCODE(0xe3, jcxz);
		OPCODE(0xe4, in_al_imm8);
		OPCODE(0xe5, in_ax_imm8);
		OPCODE(0xe6, out_al_imm8);
		OPCODE(0xe7, out_ax_imm8);
		OPCODE(0xe8, call_near);
		OPCODE(0xe9, jmp_near);
		OPCODE(0xea, jmp_far);
		OPCODE(0xeb, jmp_short);
		OPCODE(0xec, in_al_dx);
		OPCODE(0xed, in_ax_dx);
		OPCODE(0xee, out_al_dx);
		OPCODE(0xef, out_ax_dx);
		OPCODE(0xf0, lock_prefix);
		OPCODE(0xf1, unused);
		OPCODE(0xf2, repne);
		OPCODE(0xf3, rep);
		OPCODE(0xf4, hlt);
		OPCODE(0xf5, cmc);
		OPCODE(0xf6, grp3_rmw);
		OPCODE(0xf7, grp3_rmw);
		OPCODE(0xf8, clc);
		OPCODE(0xf9, stc);
		OPCODE(0xfa, cli);
		OPCODE(0xfb, sti);
		OPCODE(0xfc, cld);
		OPCODE(0xfd, std);
		OPCODE(0xfe, grp4_rm8);
		OPCODE(0xff, grp5);
	}

	return 1;
}

/*
 * ##     ## ######## ##     ##    ########        ## ##      ##
 * ###   ### ##       ###   ###    ##     ##      ##  ##  ##  ##
 * #### #### ##       #### ####    ##     ##     ##   ##  ##  ##
 * ## ### ## ######   ## ### ##    ########     ##    ##  ##  ##
 * ##     ## ##       ##     ##    ##   ##     ##     ##  ##  ##
 * ##     ## ##       ##     ##    ##    ##   ##      ##  ##  ##
 * ##     ## ######## ##     ##    ##     ## ##        ###  ###
 */

static bool dump_mem_read = CPULOG;
byte i8086_t::read_mem8(uint16_t seg, uint16_t ofs) {
	byte v = memory[0x10 * seg + ofs];
	if (dump_mem_read) {
		printf("\nr%04x:%04x = %02x (%c)\t", seg, ofs, v, isprint(v) ? v : '.');
	}

	return v;
}

uint16_t i8086_t::read_mem16(uint16_t seg, uint16_t ofs) {
	byte b0 = read_mem8(seg, ofs);
	byte b1 = read_mem8(seg, ofs+1);

	if (ofs & 1) {
		cycles += 4;
	}

	uint16_t w = (uint16_t(b0) << 0u)
	           + (uint16_t(b1) << 8u);

	return w;
}

void i8086_t::write_mem8(uint16_t seg, uint16_t ofs, byte v) {
	if (CPULOG) {
		uint32_t ea = 0x10 * seg + ofs;
		printf("wb [%05x] <- %02x\n", ea, v);
	}
	memory[0x10 * seg + ofs] = v;
}

void i8086_t::write_mem16(uint16_t seg, uint16_t ofs, uint16_t v) {
	if (CPULOG) {
		uint32_t ea = 0x10 * seg + ofs;
		printf("ww [%05x] <- %04x\n", ea, v);
	}

	if (ofs & 1) {
		cycles += 4;
	}

	memory[0x10 * seg + ofs + 0] = readlo(v);
	memory[0x10 * seg + ofs + 1] = readhi(v);
}

byte i8086_t::fetch8() {
	byte v = read_mem8(cs, ip);

	// printf("fetch8: %04x:%04x = %02x\n", cs, ip, v);

	ip += 1;
	return v;
}

uint16_t i8086_t::fetch16() {
	uint16_t w = read_mem16(cs, ip);

	// printf("fetch16: %04x:%04x = %04x\n", cs, ip, w);

	ip += 2;
	return w;
}

inline
uint16_t sext(byte v) {
	if (v & 0x80) {
		return 0xff00 | uint16_t(v);
	}
	return v;
}

void i8086_t::push(uint16_t v) {
	sp -= 2;
	write_mem16(ss, sp, v);
}

uint16_t i8086_t::pop() {
	uint16_t v = read_mem16(ss, sp);
	sp += 2;
	return v;
}

/*
 * TODO: Gather str_-functions.
 */

const char *i8086_t::str_imm(uint16_t imm) {
	static char s[16];
	uint16_t t = imm;
	while (t > 0xf) {
		t = t >> 4;
	}
	sprintf(s, "%s%xh", t > 9 ? "0" : "", imm);
	return s;
}

const char *i8086_t::str_sreg(byte reg) {
	const char *regnames = "es\0cs\0ss\0ds";
	return &regnames[3 * reg];
}

/*
 *  ######  ########  ######      ########  ########  ######    ######
 * ##    ## ##       ##    ##     ##     ## ##       ##    ##  ##    ##
 * ##       ##       ##           ##     ## ##       ##        ##
 *  ######  ######   ##   ####    ########  ######   ##   ####  ######
 *       ## ##       ##    ##     ##   ##   ##       ##    ##        ##
 * ##    ## ##       ##    ##     ##    ##  ##       ##    ##  ##    ##
 *  ######  ########  ######      ##     ## ########  ######    ######
*/

uint16_t i8086_t::read_sreg(byte sreg) {
	switch (sreg) {
		case SEG_ES: return es; break;
		case SEG_CS: return cs; break;
		case SEG_SS: return ss; break;
		case SEG_DS: return ds; break;
		default: assert(0 && "invalid sreg");
	}
}

void i8086_t::write_sreg(byte sreg, uint16_t v) {
	switch (sreg) {
		case SEG_ES: es = v; break;
		case SEG_CS: cs = v; break;
		case SEG_SS: ss = v; break;
		case SEG_DS: ds = v; break;
		default: assert(0 && "invalid sreg");
	}
}

byte i8086_t::get_sreg_ovr(byte sreg_def) {
	byte sreg = sreg_def;
	if (sreg_ovr) {
		sreg = (sreg_ovr >> 3) & 0b11;
	}
	return sreg;
}

uint16_t i8086_t::read_sreg_ovr(byte sreg_def) {
	byte sreg = get_sreg_ovr(sreg_def);
	return read_sreg(sreg);
}

void i8086_t::write_sreg_ovr(byte sreg_def, uint16_t v) {
	byte sreg = get_sreg_ovr(sreg_def);
	return write_sreg(sreg, v);
}

/*
 * ##     ##  #######  ########          ########  ##     ##
 * ###   ### ##     ## ##     ##         ##     ## ###   ###
 * #### #### ##     ## ##     ##         ##     ## #### ####
 * ## ### ## ##     ## ##     ## ####### ########  ## ### ##
 * ##     ## ##     ## ##     ##         ##   ##   ##     ##
 * ##     ## ##     ## ##     ##         ##    ##  ##     ##
 * ##     ##  #######  ########          ##     ## ##     ##
 */

void i8086_t::modrm_t::print() {
	if (!is_mem) {
		printf("%s", str_reg(reg, w));
	} else {
		if (sreg_ovr) {
			printf("%s:", str_sreg((sreg_ovr >> 3) & 0b11));
		}
		printf("[%04x]", ofs);
	}
}

i8086_t::modrm_t i8086_t::modrm_mem_sw(byte modrm, bool s, bool w) {
	byte mod = (modrm >> 6);
	byte rm  = (modrm >> 0) & 0b111;

	modrm_t res;

	res.is_mem = mod != 0b11;
	res.w = w;
	res.s = s;

	if (mod == 0b11) {
		res.reg = rm;
	} else {
		uint16_t ofs;
		res.is_mem = true;
		res.sreg_ovr = sreg_ovr;
		switch (rm) {
			case 0b000: ofs = bx + si; break;
			case 0b001: ofs = bx + di; break;
			case 0b010: ofs = bp + si; res.sreg_ovr = 0x36; break;
			case 0b011: ofs = bp + di; res.sreg_ovr = 0x36; break;
			case 0b100: ofs = si;      break;
			case 0b101: ofs = di;      break;
			case 0b110: ofs = mod ? bp : fetch16(); break;
			case 0b111: ofs = bx;      break;
		}
		switch (mod) {
			case 0b01: ofs += fetch8(); break;
			case 0b10: ofs += fetch16(); break;
		}
		res.ofs = ofs;
	}

	return res;
}

i8086_t::modrm_t i8086_t::modrm_reg_sw(byte modrm, bool s, bool w) {
	byte reg = (modrm >> 3) & 0b111;

	modrm_t res;

	res.is_mem = false;
	res.w = w;
	res.s = s;
	res.reg = reg;

	return res;
}

uint16_t i8086_t::read_modrm(i8086_t::modrm_t dst, byte sreg_def) {
	uint16_t v;

	if (!dst.is_mem) {
		v = read_reg(dst.reg, dst.w);

		if (dst.s) {
			v = sext(v);
		}
	} else {
		v = read_mem(read_sreg_ovr(sreg_def), dst.ofs, dst.w);
	}

	return v;
}

void i8086_t::write_modrm(i8086_t::modrm_t dst, uint16_t v, byte sreg_def) {
	if (!dst.is_mem) {
		if (dst.s) {
			v = sext(v);
		}

		write_reg(dst.reg, v, dst.w);
	} else {
		write_mem(read_sreg_ovr(sreg_def), dst.ofs, v, dst.w);
	}
}

const char *i8086_t::str_reg(byte reg, bool w) {
	const char *regnames = "al\0cl\0dl\0bl\0ah\0ch\0dh\0bh\0ax\0cx\0dx\0bx\0sp\0bp\0si\0di";
	byte index = (((byte)w << 3) | reg);
	return &regnames[3 * index];
}

uint16_t i8086_t::read_reg(byte reg, bool w) {
	uint16_t v;
	switch (((byte)w << 3) | reg) {
		case 0b0000: v = readlo(ax); break;
		case 0b0001: v = readlo(cx); break;
		case 0b0010: v = readlo(dx); break;
		case 0b0011: v = readlo(bx); break;
		case 0b0100: v = readhi(ax); break;
		case 0b0101: v = readhi(cx); break;
		case 0b0110: v = readhi(dx); break;
		case 0b0111: v = readhi(bx); break;
		case 0b1000: v = ax; break;
		case 0b1001: v = cx; break;
		case 0b1010: v = dx; break;
		case 0b1011: v = bx; break;
		case 0b1100: v = sp; break;
		case 0b1101: v = bp; break;
		case 0b1110: v = si; break;
		case 0b1111: v = di; break;
		default:
			assert(0 && "unreachable");
	}
	return v;
}

void i8086_t::write_reg(byte reg, uint16_t v, bool w) {
	switch (((byte)w << 3) | reg) {
		case 0b0000: writelo(ax, v); break;
		case 0b0001: writelo(cx, v); break;
		case 0b0010: writelo(dx, v); break;
		case 0b0011: writelo(bx, v); break;
		case 0b0100: writehi(ax, v); break;
		case 0b0101: writehi(cx, v); break;
		case 0b0110: writehi(dx, v); break;
		case 0b0111: writehi(bx, v); break;
		case 0b1000: ax = v; break;
		case 0b1001: cx = v; break;
		case 0b1010: dx = v; break;
		case 0b1011: bx = v; break;
		case 0b1100: sp = v; break;
		case 0b1101: bp = v; break;
		case 0b1110: si = v; break;
		case 0b1111: di = v; break;
	}
}

const char *i8086_t::str_w(bool w) {
	static const char *wnames = "byte\0word";
	return &wnames[5 * w];
}

/*
 *    ###    ##       ##     ##
 *   ## ##   ##       ##     ##
 *  ##   ##  ##       ##     ##
 * ##     ## ##       ##     ##
 * ######### ##       ##     ##
 * ##     ## ##       ##     ##
 * ##     ## ########  #######
 */

uint16_t i8086_t::alu_sw(byte func, uint16_t a, uint16_t b, bool w) {
	uint16_t res;
	uint16_t c = flags.cf;

	switch (func) {
		case ALU_ADD: res = a + b; break;
		case ALU_OR:  res = a | b; break;
		case ALU_ADC: res = a + b + c; break;
		case ALU_SBB: res = a - b - c; break;
		case ALU_AND: res = a & b; break;
		case ALU_SUB: res = a - b; break;
		case ALU_XOR: res = a ^ b; break;
		case ALU_CMP: res = a - b; break;
		default: assert(0 && "invalid alu func");
	}

	if (!w) {
		switch (func) {
			case ALU_ADD: update_flags_add8(res, a, b); break;
			case ALU_OR:  update_flags_bin8(res, a, b); break;
			case ALU_ADC: update_flags_add8(res, a, b); break;
			case ALU_SBB: update_flags_sub8(res, a, b); break;
			case ALU_AND: update_flags_bin8(res, a, b); break;
			case ALU_SUB: update_flags_sub8(res, a, b); break;
			case ALU_XOR: update_flags_bin8(res, a, b); break;
			case ALU_CMP: update_flags_sub8(res, a, b); break;
		}
	} else {
		switch (func) {
			case ALU_ADD: update_flags_add16(res, a, b); break;
			case ALU_OR:  update_flags_bin16(res, a, b); break;
			case ALU_ADC: update_flags_add16(res, a, b); break;
			case ALU_SBB: update_flags_sub16(res, a, b); break;
			case ALU_AND: update_flags_bin16(res, a, b); break;
			case ALU_SUB: update_flags_sub16(res, a, b); break;
			case ALU_XOR: update_flags_bin16(res, a, b); break;
			case ALU_CMP: update_flags_sub16(res, a, b); break;
		}
	}

	if (!w) {
		res &= 0x00ff;
	}

	return res;
}

/*
 * ######## ##          ###     ######    ######
 * ##       ##         ## ##   ##    ##  ##    ##
 * ##       ##        ##   ##  ##        ##
 * ######   ##       ##     ## ##   ####  ######
 * ##       ##       ######### ##    ##        ##
 * ##       ##       ##     ## ##    ##  ##    ##
 * ##       ######## ##     ##  ######    ######
 */

/*
 * Sign
 */

inline bool signbit8(uint16_t v) {
	return (v >> 7) & 1;
}

inline bool signbit16(uint16_t v) {
	return v >> 15;
}

/*
 * Carry
 */

inline bool cf8_add(uint16_t res, uint16_t dst, uint16_t src) {
	return readlo(res) < readlo(dst);
}

inline bool cf16_add(uint16_t res, uint16_t dst, uint16_t src) {
	return res < dst;
}

inline bool cf_w_add(uint16_t res, uint16_t dst, uint16_t src, bool w) {
	return !w ? cf8_add(res, dst, src) : cf16_add(res, dst, src);
}

inline bool cf8_sub(uint16_t res, uint16_t dst, uint16_t src) {
	return readlo(dst) < readlo(src);
}

inline bool cf16_sub(uint16_t res, uint16_t dst, uint16_t src) {
	return dst < src;
}

inline bool cf_w_sub(uint16_t res, uint16_t dst, uint16_t src, bool w) {
	return !w ? cf8_sub(res, dst, src) : cf16_sub(res, dst, src);
}

/*
 * Parity
 */

inline bool pf8(uint16_t res) {
	return (0x9669 >> ((res ^ (res >> 4)) & 0xf)) & 1;
}

inline bool pf16(uint16_t res) {
	return pf8(res);
}

inline bool pf_w(uint16_t res, bool w) {
	(void)w;
	return pf8(res);
}

/*
 * Adjust
 */

inline bool af8(uint16_t res, uint16_t dst, uint16_t src) {
	return ((res ^ src ^ dst) >> 4) & 1;
}

inline bool af16(uint16_t res, uint16_t dst, uint16_t src) {
	return af8(res, dst, src);
}

inline bool af_w(uint16_t res, uint16_t dst, uint16_t src, bool w) {
	(void)w;
	return af8(res, dst, src);
}

/*
 * Zero
 */

inline bool zf8(uint16_t res) {
	return (res & 0x00ff) == 0;
}

inline bool zf16(uint16_t res) {
	return res == 0;
}

inline bool zf_w(uint16_t res, bool w) {
	return !w ? zf8(res) : zf16(w);
}

/*
 * Sign
 */

inline bool sf8(uint16_t res) {
	return signbit8(res);
}

inline bool sf16(uint16_t res) {
	return signbit16(res);
}

inline bool sf_w(uint16_t res, bool w) {
	return !w ? sf8(res) : sf16(res);
}

/*
 * Overflow
 */

inline bool of8_add(uint16_t res, uint16_t dst, uint16_t src) {
	return signbit8((res ^ src) & (res ^ dst));
}

inline bool of16_add(uint16_t res, uint16_t dst, uint16_t src) {
	return signbit16((res ^ src) & (res ^ dst));
}

inline bool of_w_add(uint16_t res, uint16_t dst, uint16_t src, bool w) {
	return !w ? of8_add(res, dst, src) : of16_add(res, dst, src);
}

inline bool of8_sub(uint16_t res, uint16_t dst, uint16_t src) {
	return signbit8((src ^ dst) & (res ^ dst));
}

inline bool of16_sub(uint16_t res, uint16_t dst, uint16_t src) {
	return signbit16((src ^ dst) & (res ^ dst));
}

inline bool of_w_sub(uint16_t res, uint16_t dst, uint16_t src, bool w) {
	return !w ? of8_sub(res, dst, src) : of16_sub(res, dst, src);
}

void i8086_t::update_flags_add8(uint8_t res, uint8_t dst, uint8_t src) {
	flags.cf = cf8_add(res, dst, src);
	flags.pf = pf8(res);
	flags.af = af8(res, dst, src);
	flags.zf = zf8(res);
	flags.sf = sf8(res);
	flags.of = of8_add(res, dst, src);
}

void i8086_t::update_flags_sub8(uint8_t res, uint8_t dst, uint8_t src) {
	flags.cf = cf8_sub(res, dst, src);
	flags.pf = pf8(res);
	flags.af = af8(res, dst, src);
	flags.zf = zf8(res);
	flags.sf = sf8(res);
	flags.of = of8_sub(res, dst, src);
}

void i8086_t::update_flags_bin8(uint8_t res, uint8_t dst, uint8_t src) {
	flags.cf = 0;
	flags.pf = pf8(res);
	flags.zf = zf8(res);
	flags.sf = sf8(res);
	flags.af = 0; // TODO: DOSBox does this.
	flags.of = 0;
}

void i8086_t::update_flags_shf8(uint8_t res, uint8_t src, bool cf) {
	flags.cf = cf;
	flags.pf = pf8(res);
	flags.zf = zf8(res);
	flags.sf = sf8(res);
	flags.of = (res & 0x80) != (src & 0x80);
}

void i8086_t::update_flags_rot8(uint8_t res, uint8_t src, bool cf) {
	flags.cf = cf;
	flags.of = (res & 0x80) != (src & 0x80);
}

void i8086_t::update_flags_add16(uint16_t res, uint16_t dst, uint16_t src) {
	flags.cf = cf16_add(res, dst, src);
	flags.pf = pf16(res);
	flags.af = af16(res, dst, src);
	flags.zf = zf16(res);
	flags.sf = sf16(res);
	flags.of = of16_add(res, dst, src);
}

void i8086_t::update_flags_sub16(uint16_t res, uint16_t dst, uint16_t src) {
	flags.cf = cf16_sub(res, dst, src);
	flags.pf = pf16(res);
	flags.af = af16(res, dst, src);
	flags.zf = zf16(res);
	flags.sf = sf16(res);
	flags.of = of16_sub(res, dst, src);
}

void i8086_t::update_flags_bin16(uint16_t res, uint16_t dst, uint16_t src) {
	flags.cf = 0;
	flags.pf = pf16(res);
	flags.zf = zf16(res);
	flags.sf = sf16(res);
	flags.af = 0; // TODO: DOSBox does this.
	flags.of = 0;
}

void i8086_t::update_flags_shf16(uint16_t res, uint16_t src, bool cf) {
	flags.cf = cf;
	flags.pf = pf16(res);
	flags.zf = zf16(res);
	flags.sf = sf16(res);
	flags.of = (res & 0x8000) != (src & 0x8000);
}

void i8086_t::update_flags_rot16(uint16_t res, uint16_t src, bool cf) {
	flags.cf = cf;
	flags.of = (res & 0x8000) != (src & 0x8000);
}

void i8086_t::unimplemented(const char *op_name, int line) {
	printf("unimplemented! %s:%d [%02x]\n", op_name, line, op);
	dump_state();
	exit(0);
}

void i8086_t::op_unused() {
	unimplemented(__FUNCTION__, __LINE__);
}

/*
 *  #######  ########   ######   #######  ########  ########  ######
 * ##     ## ##     ## ##    ## ##     ## ##     ## ##       ##    ##
 * ##     ## ##     ## ##       ##     ## ##     ## ##       ##
 * ##     ## ########  ##       ##     ## ##     ## ######    ######
 * ##     ## ##        ##       ##     ## ##     ## ##             ##
 * ##     ## ##        ##    ## ##     ## ##     ## ##       ##    ##
 *  #######  ##         ######   #######  ########  ########  ######
 */

void i8086_t::op_alu_r_rm() {
	byte modrm = fetch8();
	bool w     = !!(op & 0b01);
	bool d     = !!(op & 0b10);
	byte func  = (op >> 3) & 0b111;

	modrm_t mem = modrm_mem_sw(modrm, false, w);
	modrm_t reg = modrm_reg_sw(modrm, false, w);

	uint16_t a = read_modrm(reg);
	uint16_t b = read_modrm(mem);
	uint16_t r = alu_sw(func, a, b, w);

	if (func != ALU_CMP) {
		write_modrm(!d ? mem : reg, r);
	}

	if (CHANIDEBUG) {
		const char *s_oper = &"add\0or\0\0adc\0sbb\0and\0sub\0xor\0cmp"[4 * func];
		printf("%s\t", s_oper);
		if (!d) {
			mem.print();
		} else {
			reg.print();
		}
		printf(", ");
		if (d) {
			mem.print();
		} else {
			reg.print();
		}
		printf("\n");
	}
}

void i8086_t::op_alu_a_imm() {
	bool w    = op & 1;
	byte func = (op >> 3) & 0b111;

	uint16_t a = read_reg(REG_AX, w);
	uint16_t b = fetch(w);
	uint16_t r = alu_sw(func, a, b, w);

	if (func != ALU_CMP) {
		write_reg(REG_AX, r, w);
	}

	if (CHANIDEBUG) {
		const char *s_oper = &"add\0or\0\0adc\0sbb\0and\0sub\0xor\0cmp"[4 * func];
		printf("%s\t%s, %s\n", s_oper, !w ? "al" : "ax", str_imm(b));
	}
}

void i8086_t::op_push_sreg() {
	byte sreg = (op >> 3) & 0b111;

	uint16_t v = read_sreg(sreg);
	push(v);

	if (CHANIDEBUG) {
		printf("push\t%s\n", str_sreg(sreg));
	}
}

void i8086_t::op_pop_sreg() {
	byte sreg = (op >> 3) & 0b111;

	uint16_t v = pop();
	write_sreg(sreg, v);

	if (CHANIDEBUG) {
		printf("pop\t%s\n", str_sreg(sreg));
	}
}

void i8086_t::op_seg_ovr_es() {
	is_prefix = true;
	sreg_ovr = op;
}

void i8086_t::op_daa() {
	uint8_t al = readlo(ax);
	bool    af = flags.af;
	bool    cf = flags.cf;

	uint8_t old_al = al;
	bool    old_cf = cf;
	cf = 0;

	if ((al & 0x0f) > 9 || af) {
		al = al + 6;
		cf = old_cf || (al < old_al);
		af = 1;
	} else {
		af = 0;
	}
	if ((old_al > 0x99) || old_cf) {
		al += 0x60;
		cf = 1;
	} else {
		cf = 0;
	}

	writelo(ax, al);
	flags.pf = pf8(al);
	flags.af = af;
	flags.zf = zf8(al);
	flags.sf = sf8(al);
}

void i8086_t::op_seg_ovr_cs() {
	is_prefix = true;
	sreg_ovr = op;
}

void i8086_t::op_das() {
	uint8_t al = readlo(ax);

	uint8_t new_al = al;
	bool    new_af = 0;
	bool    new_cf = 0;

	if ((al & 0x0f) > 9 || flags.af) {
		new_al = al - 6;
		new_cf = flags.cf || (new_al > al);
		new_af = 1;
	}
	if ((al > 0x99) || flags.cf) {
		new_al -= 0x60;
		new_cf = 1;
	}

	flags.cf = new_cf;
	flags.pf = pf8(new_al);
	flags.af = new_af;
	flags.zf = zf8(al);
	flags.sf = sf8(new_al);
	writelo(ax, new_al);
}

void i8086_t::op_seg_ovr_ss() {
	is_prefix = true;
	sreg_ovr = op;
}

void i8086_t::op_aaa() {
	unimplemented(__FUNCTION__, __LINE__);
	cycles += 4;
}

void i8086_t::op_seg_ovr_ds() {
	is_prefix = true;
	sreg_ovr = op;
}

void i8086_t::op_aas() {
	unimplemented(__FUNCTION__, __LINE__);
	cycles += 4;
}

void i8086_t::op_inc_reg() {
	byte reg = op & 0b111;

	uint16_t dst = read_reg(reg, true);
	uint16_t src = 1;
	uint16_t res = dst + 1;
	write_reg(reg, res, true);

	flags.pf = pf16(res);
	flags.af = af16(res, dst, src);
	flags.zf = zf16(res);
	flags.sf = sf16(res);
	flags.of = of16_add(res, dst, src);

	if (CHANIDEBUG) {
		printf("inc\t%s\n", str_reg(reg, true));
	}
}

void i8086_t::op_dec_reg() {
	byte reg = op & 0b111;

	uint16_t dst = read_reg(reg, true);
	uint16_t src = 1;
	uint16_t res = dst - 1;
	write_reg(reg, res, true);

	flags.pf = pf16(res);
	flags.af = af16(res, dst, src);
	flags.zf = zf16(res);
	flags.sf = sf16(res);
	flags.of = of16_sub(res, dst, src);

	if (CHANIDEBUG) {
		printf("dec\t%s\n", str_reg(reg, true));
	}
}

void i8086_t::op_push_reg() {
	byte reg = op & 0b111;

	// On 8086 'push ss' pushes the updated value of ss
	// so we can't use ::push(v)
	sp -= 2;
	uint16_t v = read_reg(reg, true);
	write_mem16(ss, sp, v);

	if (CHANIDEBUG) {
		printf("push\t%s\n", str_reg(reg, true));
	}
}

void i8086_t::op_pop_reg() {
	byte reg = op & 0b111;

	uint16_t v = pop();
	write_reg(reg, v, true);

	if (CHANIDEBUG) {
		printf("pop\t%s\n", str_reg(reg, true));
	}
}

void i8086_t::op_jcc() {
	byte cond = (op >> 1) & 0b111;
	bool neg = op & 1;
	bool r;

	int8_t inc = (int8_t)fetch8();

	switch (cond) {
		case 0b000: r = flags.of; break;
		case 0b001: r = flags.cf; break;
		case 0b010: r = flags.zf; break;
		case 0b011: r = flags.cf | flags.zf; break;
		case 0b100: r = flags.sf; break;
		case 0b101: r = flags.pf; break;
		case 0b110: r = flags.sf ^ flags.of; break;
		case 0b111: r = (flags.sf ^ flags.of) | flags.zf; break;
		default:
			assert(0 && "unreachable");
	}
	if (neg) {
		r = !r;
	}

	if (CHANIDEBUG) {
		const char *s_cond = "o\0\0b\0\0e\0\0be\0s\0\0p\0\0l\0\0le";
		printf("j%s%s\t%04xh\n", (op & 1) ? "n" : "", &s_cond[3*cond], ip + inc);
	}

	if (r) {
		ip += inc;
	}
}

void i8086_t::op_grp1_rmw_imm() {
	bool w      = !!(op & 1);
	byte modrm  = fetch8();
	byte func   = (modrm >> 3) & 0b111;
	modrm_t dst = modrm_mem_sw(modrm, false, w);
	uint16_t imm;

	switch (op & 3) { // s:w
		case 0b00: imm = fetch8(); break;
		case 0b01: imm = fetch16(); break;
		case 0b10:
		case 0b11: imm = sext(fetch8()); break;
		default:
			assert(0 && "unreachable");
	}

	uint16_t a = read_modrm(dst);
	uint16_t b = imm;
	uint16_t r = alu_sw(func, a, b, w);

	if (func != ALU_CMP) {
		write_modrm(dst, r);
	}

	if (CHANIDEBUG) {
		const char *s_oper = &"add\0or\0\0adc\0sbb\0and\0sub\0xor\0cmp"[4 * func];
		printf("%s\t%s ", s_oper, str_w(w));
		dst.print();
		printf(", %s\n", str_imm(imm));
	}
}

void i8086_t::op_test_rm8_r8() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_test_rm16_r16() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_xchg_rm_r() {
	bool w     = !!(op & 0b01);
	byte modrm = fetch8();

	modrm_t mem = modrm_mem_sw(modrm, false, w);
	modrm_t reg = modrm_reg_sw(modrm, false, w);

	uint16_t a = read_modrm(mem);
	uint16_t b = read_modrm(reg);

	write_modrm(reg, a);
	write_modrm(mem, b);

	if (CHANIDEBUG) {
		printf("xchg\t");
		mem.print();
		printf(", ");
		reg.print();
		printf("\n");
	}
}

void i8086_t::op_mov_rm_r() {
	bool w     = !!(op & 0b01);
	bool d     = !!(op & 0b10);
	byte modrm = fetch8();

	modrm_t mem = modrm_mem_sw(modrm, false, w);
	modrm_t reg = modrm_reg_sw(modrm, false, w);

	if (!d) {
		write_modrm(mem, read_modrm(reg));
	} else {
		write_modrm(reg, read_modrm(mem));
	}

	if (CHANIDEBUG) {
		printf("mov\t");
		if (!d) {
			mem.print();
			printf(", ");
			reg.print();
		} else {
			reg.print();
			printf(", ");
			mem.print();
		}
		printf("\n");
	}
}

void i8086_t::op_mov_rm16_sreg() {
	bool d = !!(op & 2);

	byte modrm = fetch8();
	byte sreg = (modrm >> 3) & 0b111;

	if (!d) {
		modrm_t dst = modrm_mem_sw(modrm, false, true);
		write_modrm(dst, read_sreg(sreg));

		if (CHANIDEBUG) {
			printf("mov\t");
			dst.print();
			printf(", %s\n", str_sreg(sreg));
		}
	} else {
		modrm_t src = modrm_mem_sw(modrm, false, true);
		write_sreg(sreg, read_modrm(src));

		if (CHANIDEBUG) {
			printf("mov\t%s, ", str_sreg(sreg));
			src.print();
			printf("\n");
		}
	}
}

void i8086_t::op_lea_r16_m16() {
	byte modrm = fetch8();
	modrm_t src = modrm_mem_sw(modrm, false, true);
	modrm_t dst = modrm_reg_sw(modrm, false, true);
	write_modrm(dst, src.ofs);

	if (CHANIDEBUG) {
		printf("mov\t");
		dst.print();
		printf(", ");
		src.print();
		printf("\n");
	}
}

void i8086_t::op_pop_rm16() {
	byte modrm = fetch8();

	modrm_t dst = modrm_mem_sw(modrm, false, true);
	write_modrm(dst, pop());

	if (CHANIDEBUG) {
		printf("pop\t");
		dst.print();
		printf("\n");
	}
}

void i8086_t::op_xchg_ax_r() {
	byte reg = op & 0b111;

	uint16_t tmp = read_reg(reg, true);
	write_reg(reg, ax, true);
	ax = tmp;
}

void i8086_t::op_cbw() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_cwd() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_call_far() {
	uint16_t seg = fetch16();
	uint16_t ofs = fetch16();

	push(cs);
	push(ip);

	cs = seg;
	ip = ofs;

	if (CHANIDEBUG) {
		printf("call far %04x:%04x\n", seg, ofs);
	}
}

void i8086_t::op_wait() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_pushf() {
	uint16_t psw = 0;

	psw |= flags.of  ? (1 << 11) : 0;
	psw |= flags.df  ? (1 << 10) : 0;
	psw |= flags.if_ ? (1 <<  9) : 0;
	psw |= flags.tf  ? (1 <<  8) : 0;
	psw |= flags.sf  ? (1 <<  7) : 0;
	psw |= flags.zf  ? (1 <<  6) : 0;
	psw |= flags.af  ? (1 <<  4) : 0;
	psw |= flags.pf  ? (1 <<  2) : 0;
	psw |= flags.cf  ? (1 <<  0) : 0;

	push(psw);

	if (CHANIDEBUG) {
		printf("pushf\n");
	}
}

void i8086_t::op_popf() {
	uint16_t psw = pop();

	flags.of  = (psw & (1 << 11)) ? 1 : 0;
	flags.df  = (psw & (1 << 10)) ? 1 : 0;
	flags.if_ = (psw & (1 <<  9)) ? 1 : 0;
	flags.tf  = (psw & (1 <<  8)) ? 1 : 0;
	flags.sf  = (psw & (1 <<  7)) ? 1 : 0;
	flags.zf  = (psw & (1 <<  6)) ? 1 : 0;
	flags.af  = (psw & (1 <<  4)) ? 1 : 0;
	flags.pf  = (psw & (1 <<  2)) ? 1 : 0;
	flags.cf  = (psw & (1 <<  0)) ? 1 : 0;

	if (CHANIDEBUG) {
		printf("popf\n");
	}
}

void i8086_t::op_sahf() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_lahf() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_mov_a_m() {
	bool w = !!(op & 1);
	bool d = !!(op & 2);
	uint16_t seg = read_sreg_ovr(SEG_DS);
	uint16_t ofs = fetch16();
	uint16_t v;
	if (!d) {
		v = read_mem(seg, ofs, w);
		write_reg(REG_AX, v, w);
	} else {
		v = read_reg(REG_AX, w);
		write_mem(seg, ofs, v, w);
	}

	if (CHANIDEBUG) {
		if (!d) {
			printf("mov\t%s, [%s]\n", str_reg(REG_AX, w), str_imm(ofs));
		} else {
			printf("mov\t[%s], %s\n", str_imm(ofs), str_reg(REG_AX, w));
		}
	}
}

void i8086_t::op_movs() {
	bool     w        = !!(op & 1);
	int      delta    = strop_delta(w);
	byte     src_sreg = get_sreg_ovr(SEG_DS);
	uint16_t src_seg  = read_sreg(src_sreg);

	if (repmode == REP_NONE) {
		goto inst;
	}
repeat:
	if (cx == 0) {
		return;
	}
	// TODO: Service interrupts
	--cx;
inst:
	write_mem(es, di, read_mem(src_seg, si, w), w);

	si += delta;
	di += delta;

	if (repmode != REP_NONE) {
		goto repeat;
	}
}

void i8086_t::op_cmps() {
	bool     w      = !!(op & 1);
	int      delta  = strop_delta(w);
	uint16_t di_seg = read_sreg_ovr(SEG_ES);
	uint16_t si_seg = read_sreg(SEG_DS);

	if (CHANIDEBUG) {
		if (repmode == REP_REP) {
			printf("rep ");
		} else if (repmode == REP_REPNE) {
			printf("repne ");
		}
		printf("cmps %s ", str_w(w));
		printf("%04x:%04x, " , di_seg, di);
		printf("%04x:%04x\n", si_seg, si);
	}

	if (repmode == REP_NONE) {
		goto inst;
	}
repeat:
	if (cx == 0) {
		return;
	}
	// TODO: Service interrupts
	--cx;
inst:
	byte a = read_mem(di_seg, di, w);
	byte b = read_mem(si_seg, si, w);
	si += delta;
	di += delta;

	alu_sw(ALU_SUB, b, a, w);

	if (CHANIDEBUG) {
		printf(" %c <=> %c, zf=%d\n", a, b, flags.zf);
	}

	if (repmode == REP_REP && flags.zf) {
		goto repeat;
	}
	if (repmode == REP_REPNE && !flags.zf) {
		goto repeat;
	}
}

void i8086_t::op_test_a_imm() {
	// unimplemented(__FUNCTION__, __LINE__);
	bool w = op & 1;
	uint16_t imm = fetch(w);

	uint16_t a = read_reg(REG_AX, w);
	alu_sw(ALU_AND, a, imm, w);

	if (CHANIDEBUG) {
		printf("test\ta%c, ", !w ? 'l' : 'x');
		str_imm(imm);
	}
}

void i8086_t::op_stos() {
	bool     w     = op & 1;
	uint16_t v     = read_reg(REG_AX, w);
	int      delta = strop_delta(w);

	if (repmode == REP_NONE) {
		goto inst;
	}
repeat:
	if (cx == 0) {
		return;
	}
	// TODO: Service interrupts
	--cx;
inst:
	write_mem(read_sreg_ovr(SEG_ES), di, v, w);
	di += delta;

	if (repmode != REP_NONE) {
		goto repeat;
	}

	if (CHANIDEBUG) {
		printf("stos%c\n", !w ? 'b' : 'w');
	}
}

void i8086_t::op_lods() {
	bool     w     = op & 1;
	uint16_t v     = read_mem(read_sreg_ovr(SEG_DS), si, w);
	int      delta = strop_delta(w);

	if (repmode == REP_NONE) {
		goto inst;
	}
repeat:
	if (cx == 0) {
		return;
	}
	// TODO: Service interrupts
	--cx;
inst:
	write_reg(REG_AX, v, w);
	si += delta;

	if (repmode != REP_NONE) {
		goto repeat;
	}

	if (CHANIDEBUG) {
		printf("lods%c\n", !w ? 'b' : 'w');
	}
}

void i8086_t::op_scas() {
	unimplemented(__FUNCTION__, __LINE__);
	// bool w   = (op >> 3) & 1;
}

void i8086_t::op_mov_reg_imm() {
	byte reg = op & 0b111;
	bool w   = (op >> 3) & 1;
	uint16_t imm = !w ? fetch8() : fetch16();
	write_reg(reg, imm, w);

	if (CHANIDEBUG) {
		printf("mov\t%s, %s\n", str_reg(reg, w), str_imm(imm));
	}
}

void i8086_t::op_ret_imm16_intraseg() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_ret_intraseg() {
	ip = pop();

	if (CHANIDEBUG) {
		printf("retn\n");
	}
}

void i8086_t::op_les_r16_m16() {
	byte modrm = fetch8();
	modrm_t src = modrm_mem_sw(modrm, false, true);
	modrm_t dst = modrm_reg_sw(modrm, false, true);

	assert(src.is_mem);

	write_modrm(dst, read_modrm(src));
	src.ofs += 2;
	es = read_modrm(src);

	if (CHANIDEBUG) {
		printf("les\t");
		dst.print();
		printf(", ");
		src.print();
		printf("\n");
	}
}

void i8086_t::op_lds_r16_m16() {
	byte modrm = fetch8();
	modrm_t src = modrm_mem_sw(modrm, false, true);
	modrm_t dst = modrm_reg_sw(modrm, false, true);

	assert(src.is_mem);

	write_modrm(dst, read_modrm(src));
	src.ofs += 2;
	ds = read_modrm(src);

	if (CHANIDEBUG) {
		printf("lds\t");
		dst.print();
		printf(", ");
		src.print();
		printf("\n");
	}
}

void i8086_t::op_mov_m_imm() {
	bool w = op & 1;
	byte modrm = fetch8();
	modrm_t dst = modrm_mem_sw(modrm, false, w);
	uint16_t imm = fetch(w);

	if (CHANIDEBUG) {
		printf("mov\t%s ptr ", str_w(w));
		dst.print();
		printf(", %s\n", str_imm(imm));
	}

	write_modrm(dst, imm);
}

void i8086_t::op_ret_imm16_interseg() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_ret_interseg() {
	ip = pop();
	cs = pop();

	if (CHANIDEBUG) {
		printf("RETF\n");
	}
}

void i8086_t::op_int_3() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_int_imm8() {
	byte imm = fetch8();

	uint16_t psw = 0;

	psw |= flags.of  ? (1 << 11) : 0;
	psw |= flags.df  ? (1 << 10) : 0;
	psw |= flags.if_ ? (1 <<  9) : 0;
	psw |= flags.tf  ? (1 <<  8) : 0;
	psw |= flags.sf  ? (1 <<  7) : 0;
	psw |= flags.zf  ? (1 <<  6) : 0;
	psw |= flags.af  ? (1 <<  4) : 0;
	psw |= flags.pf  ? (1 <<  2) : 0;
	psw |= flags.cf  ? (1 <<  0) : 0;
	(void)psw;

	// sp -= 2;
	// write_mem16(ss, sp, psw & 0x0fff);

	// sp -= 2;
	// write_mem16(ss, sp, cs);

	// sp -= 2;
	// write_mem16(ss, sp, ip);

	if (CHANIDEBUG) {
		printf("int\t%x (ax=%04x)\n", imm, ax);
	}

	machine->handle_software_interrupt(imm);

	// sp += 6;
}

void i8086_t::op_into() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_iret() {
	unimplemented(__FUNCTION__, __LINE__);
}

inline void rolb(uint16_t &v, byte n = 1) {
	while (n--) {
		v = (v << 1) | ((v & 0x80) ? 0x01 : 0x00);
	}
}

inline void rorb(uint16_t &v, byte n = 1) {
	while (n--) {
		v = ((v & 0x01) ? 0x80 : 0x00) | (v >> 1);
	}
}

inline void rclb(uint16_t &v, bool &cf, byte n = 1) {
	byte new_cf;
	while (n--) {
		new_cf = !!(v & 0x80);
		v = (v << 1) | (cf ? 0x01 : 0x00);
		cf = new_cf;
	}
}

inline void rcrb(uint16_t &v, bool &cf, byte n = 1) {
	byte new_cf;
	while (n--) {
		new_cf = !!(v & 0x01);
		v = (cf ? 0x80 : 0x00) | (v >> 1);
		cf = new_cf;
	}
}

inline void shlb(uint16_t &v, bool &cf, byte n = 1) {
	while (n--) {
		cf = !!(v & 0x80);
		v = (v << 1) & 0xff;
	}
}

inline void shrb(uint16_t &v, bool &cf, byte n = 1) {
	while (n--) {
		cf = v & 1;
		v = (v >> 1);
	}
}

inline void salb(uint16_t &v, bool &cf, byte n = 1) {
	while (n--) {
		cf = !!(v & 0x80);
		v = (v << 1) & 0xff;
	}
}

inline void sarb(uint16_t &v, bool &cf, byte n = 1) {
	while (n--) {
		cf = v & 1;
		v = (v & 0x80) | (v >> 1);
	}
}

inline void rolw(uint16_t &v, byte n = 1) {
	while (n--) {
		v = (v << 1) | ((v & 0x80) ? 0x01 : 0x00);
	}
}

inline void rorw(uint16_t &v, byte n = 1) {
	while (n--) {
		v = ((v & 0x0001) ? 0x8000 : 0x0000) | (v >> 1);
	}
}

inline void rclw(uint16_t &v, bool &cf, byte n = 1) {
	byte new_cf;
	while (n--) {
		new_cf = !!(v & 0x8000);
		v = (v << 1) | (cf ? 0x0001 : 0x0000);
		cf = new_cf;
	}
}

inline void rcrw(uint16_t &v, bool &cf, byte n = 1) {
	byte new_cf;
	while (n--) {
		new_cf = !!(v & 0x0001);
		v = (cf ? 0x8000 : 0x0000) | (v >> 1);
		cf = new_cf;
	}
}

inline void shlw(uint16_t &v, bool &cf, byte n = 1) {
	while (n--) {
		cf = !!(v & 0x8000);
		v = (v << 1);
	}
}

inline void shrw(uint16_t &v, bool &cf, byte n = 1) {
	while (n--) {
		cf = v & 1;
		v = (v >> 1);
	}
}

inline void salw(uint16_t &v, bool &cf, byte n = 1) {
	while (n--) {
		cf = !!(v & 0x80);
		v = (v << 1);
	}
}

inline void sarw(uint16_t &v, bool &cf, byte n = 1) {
	while (n--) {
		cf = v & 1;
		v = (v & 0x8000) | (v >> 1);
	}
}

void i8086_t::op_grp2_rmw() {
	bool v = !!(op & 2);
	bool w = !!(op & 1);
	byte modrm  = fetch8();
	byte func   = (modrm >> 3) & 0b111;
	modrm_t dst = modrm_mem_sw(modrm, false, w);

	byte n = v ? readlo(cx) : 1;

	// TODO: DOSBox doesn't update flags if n = 0
	if (!n) {
		return;
	}

	uint16_t src = read_modrm(dst);
	uint16_t res = src;
	bool cf = flags.cf;
	if (!w) {
		switch (func) {
			case 0: rolb(res, n);     break;
			case 1: rorb(res, n);     break;
			case 2: rclb(res, cf, n); break;
			case 3: rcrb(res, cf, n); break;
			case 4: shlb(res, cf, n); break;
			case 5: shrb(res, cf, n); break;
			case 6: salb(res, cf, n); break;
			case 7: sarb(res, cf, n); break;
		}
		if (!(func & 0b100)) {
			update_flags_rot8(res, src, cf);
		} else {
			update_flags_shf8(res, src, cf);
		}
	} else {
		switch (func) {
			case 0: rolw(res, n);     break;
			case 1: rorw(res, n);     break;
			case 2: rclw(res, cf, n); break;
			case 3: rcrw(res, cf, n); break;
			case 4: shlw(res, cf, n); break;
			case 5: shrw(res, cf, n); break;
			case 6: salw(res, cf, n); break;
			case 7: sarw(res, cf, n); break;
		}
		if (!(func & 0b100)) {
			update_flags_rot16(res, src, cf);
		} else {
			update_flags_shf16(res, src, cf);
		}
	}

	write_modrm(dst, res);

	if (CHANIDEBUG) {
		const char *s_oper = &"rol\0ror\0rcl\0rcr\0shl\0shr\0sal\0sar"[4 * func];
		printf("%s\t", s_oper);
		dst.print();
		if (v) {
			printf(", cl");
		}
		printf("\n");
	}
}

void i8086_t::op_aam() {
	unimplemented(__FUNCTION__, __LINE__);
	cycles += 83;
}

void i8086_t::op_aad() {
	unimplemented(__FUNCTION__, __LINE__);
	cycles += 60;
}

void i8086_t::op_xlat() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_esc() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_loopnz() {
	int8_t inc = (int8_t)fetch8();

	if (CHANIDEBUG) {
		printf("loopnz\t%04x\n", ip + inc);
	}

	if (--cx != 0 && !flags.zf) {
		ip += inc;
	}
}

void i8086_t::op_loopz() {
	int8_t inc = (int8_t)fetch8();

	if (CHANIDEBUG) {
		printf("loopnz\t%04x\n", ip + inc);
	}

	if (--cx != 0 && flags.zf) {
		ip += inc;
	}
}

void i8086_t::op_loop() {
	int8_t inc = (int8_t)fetch8();

	if (CHANIDEBUG) {
		printf("loop\t%04x\n", ip + inc);
	}

	if (--cx != 0) {
		ip += inc;
	}
}

void i8086_t::op_jcxz() {
	int8_t inc = (int8_t)fetch8();
	if (!cx) {
		ip += inc;
	}
}

void i8086_t::op_in_al_imm8() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_in_ax_imm8() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_out_al_imm8() {
	byte imm = fetch8();
	(void)imm;

	if (CHANIDEBUG) {
		printf("out\tal, %s\n", str_imm(imm));
	}
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_out_ax_imm8() {
	uint16_t imm = fetch16();
	(void)imm;

	if (CHANIDEBUG) {
		printf("out\tax, %s\n", str_imm(imm));
	}
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_call_near() {
	int16_t inc = (int16_t)fetch16();

	sp -= 2;
	write_mem16(ss, sp, ip);

	if (CHANIDEBUG) {
		printf("call\t%04x\n", (uint16_t)(ip + inc));
	}

	ip += inc;
}

void i8086_t::op_jmp_near() {
	int16_t inc = (int16_t)fetch16();

	if (CHANIDEBUG) {
		printf("jmp\t%04x\n", ip + inc);
	}

	ip += inc;
}

void i8086_t::op_jmp_far() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_jmp_short() {
	int8_t inc = (int8_t)fetch8();

	if (CHANIDEBUG) {
		printf("jmp\t%04x\n", ip + inc);
	}

	ip += inc;
}

void i8086_t::op_in_al_dx() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_in_ax_dx() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_out_al_dx() {
	unimplemented(__FUNCTION__, __LINE__);
	// uint16_t port  = dx;
	// byte     value = al;
	// printf("out\t%xh, %xh\n", port, value);
}

void i8086_t::op_out_ax_dx() {
	unimplemented(__FUNCTION__, __LINE__);
	// uint16_t port  = dx;
	// uint16_t value = ax;
	// printf("out\t%xh, %xh\n", port, value);
}

void i8086_t::op_lock_prefix() {
	// unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_repne() {
	is_prefix = true;
	repmode = REP_REPNE;
}

void i8086_t::op_rep() {
	is_prefix = true;
	repmode = REP_REP;
}

void i8086_t::op_hlt() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_cmc() {
	flags.cf = !flags.cf;

	if (CHANIDEBUG) {
		printf("cmc\n");
	}
}

void i8086_t::op_grp3_rmw() {
	bool w = !!(op & 1);
	byte modrm  = fetch8();
	byte func   = (modrm >> 3) & 0b111;
	modrm_t mem = modrm_mem_sw(modrm, false, w);
	uint16_t res, src;

	switch (func) {
		case 0b000: // test
		case 0b001: // test
			src = fetch(w);
			(void)src;
			if (CHANIDEBUG) {
				printf("test\t");
				printf(!w ? "%02xh" : "%04xh", src);
				printf("\n");
			}
			unimplemented(__FUNCTION__, __LINE__);
			break;
		case 0b010: // not
			if (CHANIDEBUG) {
				printf("not\t");
				mem.print();
				printf("\n");
			}
			unimplemented(__FUNCTION__, __LINE__);
			break;
		case 0b011: // neg
			src = read_modrm(mem);
			res = alu_sw(ALU_SUB, 0, src, w);
			write_modrm(mem, res);

			if (CHANIDEBUG) {
				printf("neg\t");
				mem.print();
				printf("\n");
			}
			break;
		case 0b100: // mul
			src = read_modrm(mem);
			if (!w) {
				ax = readlo(ax) * src;
				flags.of = flags.cf = readhi(ax) != 0;
			} else {
				uint32_t tmp = ax * src;
				dx = tmp >> 16;
				ax = tmp & 0xffff;
				flags.of = flags.cf = dx != 0;
			}
			flags.zf = ax == 0; // TODO: DOSBox defines zf, spec says undefined.

			if (CHANIDEBUG) {
				printf("mul\t");
				mem.print();
				printf("\n");
			}
			break;
		case 0b101: // imul
			if (CHANIDEBUG) {
				printf("imul\t");
				mem.print();
				printf("\n");
			}
			unimplemented(__FUNCTION__, __LINE__);
			break;
		case 0b110: // div
			src = read_modrm(mem);
			assert(src && "TODO: Implement div trap");

			if (!w) {
				uint16_t quotient  = ax / src;
				uint16_t remainder = ax % src;
				assert(quotient <= 0xff && "TODO: Implement div trap");
				writehi(ax, quotient);
				writelo(ax, remainder);
			} else {
				uint32_t dividend  = (((uint32_t)dx) << 16) + (uint32_t)ax;
				uint16_t quotient  = dividend / src;
				uint16_t remainder = dividend % src;
				ax = quotient;
				dx = remainder;
			}

			if (CHANIDEBUG) {
				printf("div\t");
				mem.print();
				printf("\n");
			}
			break;
		case 0b111: // idiv
			if (CHANIDEBUG) {
				printf("idiv\t");
				mem.print();
				printf("\n");
			}
			unimplemented(__FUNCTION__, __LINE__);
			break;
	}
}

void i8086_t::op_clc() {
	flags.cf = false;

	if (CHANIDEBUG) {
		printf("clc\n");
	}
}

void i8086_t::op_stc() {
	flags.cf = true;

	if (CHANIDEBUG) {
		printf("stc\n");
	}
}

void i8086_t::op_cli() {
	flags.if_ = false;

	if (CHANIDEBUG) {
		printf("cli\n");
	}
}

void i8086_t::op_sti() {
	flags.if_ = true;

	if (CHANIDEBUG) {
		printf("sti\n");
	}
}

void i8086_t::op_cld() {
	flags.df = false;

	if (CHANIDEBUG) {
		printf("cld\n");
	}
}

void i8086_t::op_std() {
	flags.df = true;

	if (CHANIDEBUG) {
		printf("std\n");
	}
}

void i8086_t::op_grp4_rm8() {
	byte modrm = fetch8();
	byte subop = (modrm >> 3) & 0b111;
	uint16_t src, dst, res;
	modrm_t mem;

	if (modrm == 0xa5) {
		printf("HOOK");
		dump_state();
		exit(0);
	}

	switch (subop) {
		case 0b000: // inc
			mem = modrm_mem_sw(modrm, false, false);
			dst = read_modrm(mem);
			src = 1;
			res = dst + src;
			write_modrm(mem, res);

			flags.pf = pf8(res);
			flags.af = af8(res, dst, src);
			flags.zf = zf8(res);
			flags.sf = sf8(res);
			flags.of = of8_add(res, dst, src);

			if (CHANIDEBUG) {
				printf("inc\t");
				mem.print();
				printf("\n");
			}
			break;
		case 0b001: // dec
			mem = modrm_mem_sw(modrm, false, false);
			dst = read_modrm(mem);
			src = 1;
			res = dst - src;
			write_modrm(mem, res);

			flags.pf = pf8(res);
			flags.af = af8(res, dst, src);
			flags.zf = zf8(res);
			flags.sf = sf8(res);
			flags.of = of8_sub(res, dst, src);

			if (CHANIDEBUG) {
				printf("dec\t");
				mem.print();
				printf("\n");
			}
			break;
	}
}

void i8086_t::op_grp5() {
	byte modrm = fetch8();
	byte subop = (modrm >> 3) & 0b111;
	modrm_t mem = modrm_mem_sw(modrm, false, true);
	switch (subop) {
		case 0b010: {
			uint16_t ofs = read_modrm(mem);

			push(cs);
			push(ip);

			ip = ofs;

			if (CHANIDEBUG) {
				printf("call \t");
				mem.print();
				printf("\n");
			}
			break;
		}
		case 0b011: {
			assert(mem.is_mem && "TODO: Illegal opcode");
			uint16_t ofs = read_modrm(mem);
			mem.ofs += 2;
			uint16_t seg = read_modrm(mem);

			push(cs);
			push(ip);

			cs = seg;
			ip = ofs;

			if (CHANIDEBUG) {
				printf("call far\t");
				mem.print();
				printf("\n");
			}
			break;
		}
		case 0b110: {
			uint16_t v = read_modrm(mem);
			push(v);

			if (CHANIDEBUG) {
				printf("push\t");
				mem.print();
				printf("\n");
			}
			break;
		}
		default:
			printf("op_grp sup op %x unimplemented\n", subop);
			unimplemented(__FUNCTION__, __LINE__);
	}
}
