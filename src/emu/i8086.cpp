#include "i8086.h"

#include "ibm5160.h"

#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <map>
#include <utility>
#include <vector>

struct call_stack_entry_t {
	i8086_addr_t from;
	i8086_addr_t to;
	bool         is_int;
};

std::vector<call_stack_entry_t> call_stack;

void dump_call_stack() {
	printf("Callstack:\n");
	for (size_t i = 0; i < call_stack.size(); i++) {
		printf("\t%3zu: %04x:%04x -> %04x:%04x%s\n", i,
			call_stack[i].from.seg, call_stack[i].from.ofs,
			call_stack[i].to.seg, call_stack[i].to.ofs,
			call_stack[i].is_int ? " - interrupt" : ""
		);
	}
	printf("\n");
}

i8086_t::i8086_t() {
	reset();
}

void i8086_t::reset() {
	is_prefix = false;
	sreg_ovr = 0;
	repmode = REP_NONE;

	int_delay = false;
	int_nmi = false;
	int_intr = false;

	flags = 0x0002;

	ip = 0xfff0;
	cs = 0xf000;
	ds = 0x0000;
	ss = 0x0000;
	es = 0x0000;

	ax = 0x0000;
	cx = 0x0000;
	dx = 0x0000;
	bx = 0x0000;

	sp = 0x0000;
	bp = 0x0000;
	di = 0x0000;
	si = 0x0000;
}

uint64_t i8086_t::next_cycles() {
	return UINT64_MAX;
}

uint64_t i8086_t::run_cycles(uint64_t cycles) {
	uint64_t actual_cycles = 0;
	while (actual_cycles < cycles) {
		actual_cycles += step();
	}
	return actual_cycles;
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

	printf("O%d ", get_of());
	printf("D%d ", get_df());
	printf("I%d ", get_if());
	printf("T%d ", get_tf());
	printf("S%d ", get_sf());
	printf("Z%d ", get_zf());
	printf("A%d ", get_af());
	printf("P%d ", get_pf());
	printf("C%d\n",get_cf());
	printf("\n");
}

void i8086_t::log_state() {
	printf("%10d: ", instr_count);
	printf("%04X:%04X  ", log_cs, log_ip);
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
	printf(" C%d", get_cf());
	printf(" Z%d", get_zf());
	printf(" S%d", get_sf());
	printf(" O%d", get_of());
	printf(" I%d", get_if());
	printf(" F%04X", get_log_flags());
	printf("\n");
}

void i8086_t::set_callback_base(uint16_t callback_base_seg) {
	callback_base_addr = i8086_addr_t { callback_base_seg, 0 };
	callback_next_addr = callback_base_addr;
}

i8086_addr_t i8086_t::install_callback(uint16_t seg, uint16_t ofs, callback_t callback) {
	i8086_addr_t addr = callback_next_addr;

	uint16_t callback_id = callbacks.size();
	callbacks.push_back(callback);

	mem_write8 (addr.seg, addr.ofs + 0, 0xfe);
	mem_write8 (addr.seg, addr.ofs + 1, 0x38);
	mem_write16(addr.seg, addr.ofs + 2, callback_id);

	mem_write16(seg, ofs + 0, addr.ofs);
	mem_write16(seg, ofs + 2, addr.seg);

	callback_next_addr.ofs += 4;
	if (callback_next_addr.ofs == 0) {
		callback_next_addr.seg += 0x100;
	}
	return addr;
}

uint32_t i8086_t::step() {
	sreg_ovr = 0;
	repmode = REP_NONE;
	uint32_t cycles = 0;

	if (cs > 0 && cs < 0xf000) {
		log_cs = cs;
		log_ip = ip;
	}

	if (!int_delay) {
		if (int_nmi) {
			int_nmi = false;
			call_int(2);
			return cycles;
		}
		if (int_intr) {
			int_intr = false;
			call_int(int_number);
			return cycles;
		}
	}
	int_delay = false;

	op_ip = ip;
	do {
		op = fetch8();
		is_prefix = false;
		cycles += dispatch();
	} while (is_prefix);

	if (cs < 0xf000) {
		instr_count++;
	}

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
		OPCODE(0x0f, pop_sreg);
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
		OPCODE(0x84, test_rm_r);
		OPCODE(0x85, test_rm_r);
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
 * #### ##    ## ######## ######## ########  ########  ##     ## ########  ########  ######
 *  ##  ###   ##    ##    ##       ##     ## ##     ## ##     ## ##     ##    ##    ##    ##
 *  ##  ####  ##    ##    ##       ##     ## ##     ## ##     ## ##     ##    ##    ##
 *  ##  ## ## ##    ##    ######   ########  ########  ##     ## ########     ##     ######
 *  ##  ##  ####    ##    ##       ##   ##   ##   ##   ##     ## ##           ##          ##
 *  ##  ##   ###    ##    ##       ##    ##  ##    ##  ##     ## ##           ##    ##    ##
 * #### ##    ##    ##    ######## ##     ## ##     ##  #######  ##           ##     ######
*/

void i8086_t::raise_nmi() {
	int_nmi = true;
}

void i8086_t::raise_intr(byte num) {
	int_intr = true;
	int_number = num;
}

void i8086_t::call_int(byte num) {
	uint16_t int_ip = mem_read16(0, 4 * num);
	uint16_t int_cs = mem_read16(0, 4 * num + 2);

	push(flags);
	push(cs);
	push(ip);

	set_if(false);
	set_tf(false);

	call_stack.push_back({{cs, op_ip}, {int_cs, int_ip}, true});

	cs = int_cs;
	ip = int_ip;
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

byte i8086_t::mem_read8(uint16_t seg, uint16_t ofs) {
	uint32_t ea = 0x10 * seg + ofs;

	byte v = read(MEM, ea, W8);

	return v;
}

uint16_t i8086_t::mem_read16(uint16_t seg, uint16_t ofs) {
	uint32_t ea = 0x10 * seg + ofs;

	uint16_t v = read(MEM, ea, W16);

	if (ofs & 1) {
		cycles += 4;
	}

	return v;
}

void i8086_t::mem_write8(uint16_t seg, uint16_t ofs, byte v) {
	uint32_t ea = 0x10 * seg + ofs;

	write(MEM, ea, W8, v);
}

void i8086_t::mem_write16(uint16_t seg, uint16_t ofs, uint16_t v) {
	uint32_t ea = 0x10 * seg + ofs;

	write(MEM, ea, W16, v);

	if (ofs & 1) {
		cycles += 4;
	}
}

byte i8086_t::fetch8() {
	byte v = mem_read8(cs, ip);

	ip += 1;

	return v;
}

uint16_t i8086_t::fetch16() {
	uint16_t w = mem_read16(cs, ip);

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

inline
uint32_t sext16(uint16_t v) {
	if (v & 0x8000) {
		return 0xffff0000 | uint32_t(v);
	}
	return v;
}

void i8086_t::push(uint16_t v) {
	sp -= 2;
	mem_write16(ss, sp, v);
}

uint16_t i8086_t::pop() {
	uint16_t v = mem_read16(ss, sp);
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
		default: break;
	}
	assert(0 && "invalid sreg");
	return 0;
}

void i8086_t::write_sreg(byte sreg, uint16_t v) {
	switch (sreg) {
		case SEG_ES: es = v; return;
		case SEG_CS: cs = v; return;
		case SEG_SS: ss = v; return;
		case SEG_DS: ds = v; return;
		default: break;
	}
	assert(0 && "invalid sreg");
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
		if (sreg != SEG_DS) {
			printf("%s:", str_sreg(sreg));
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

		// If BP is the base register, default segment to SS.
		switch (rm) {
			case 0b000: res.sreg = SEG_DS; break;
			case 0b001: res.sreg = SEG_DS; break;
			case 0b010: res.sreg = SEG_SS; break;
			case 0b011: res.sreg = SEG_SS; break;
			case 0b100: res.sreg = SEG_DS; break;
			case 0b101: res.sreg = SEG_DS; break;
			case 0b110: res.sreg = mod ? SEG_SS : SEG_DS; break;
			case 0b111: res.sreg = SEG_DS; break;
		}

		switch (rm) {
			case 0b000: ofs = bx + si; break;
			case 0b001: ofs = bx + di; break;
			case 0b010: ofs = bp + si; break;
			case 0b011: ofs = bp + di; break;
			case 0b100: ofs = si;      break;
			case 0b101: ofs = di;      break;
			case 0b110: ofs = mod ? bp : fetch16(); break;
			case 0b111: ofs = bx;      break;
		}
		switch (mod) {
			case 0b01: ofs += sext(fetch8()); break;
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

uint16_t i8086_t::read_modrm(i8086_t::modrm_t dst) {
	uint16_t v;

	if (!dst.is_mem) {
		v = read_reg(dst.reg, dst.w);

		if (dst.s) {
			v = sext(v);
		}
	} else {
		v = mem_read(read_sreg_ovr(dst.sreg), dst.ofs, dst.w);
	}

	return v;
}

void i8086_t::write_modrm(i8086_t::modrm_t dst, uint16_t v) {
	if (!dst.is_mem) {
		if (dst.s) {
			v = sext(v);
		}

		write_reg(dst.reg, v, dst.w);
	} else {
		mem_write(read_sreg_ovr(dst.sreg), dst.ofs, v, dst.w);
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

uint16_t i8086_t::alu_w(byte func, uint16_t a, uint16_t b, bool w) {
	uint16_t res;
	uint16_t c = !!(flags & FLAG_CF);

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
			case ALU_ADC: update_flags_add8(res, a, b, c); break;
			case ALU_SBB: update_flags_sub8(res, a, b, c); break;
			case ALU_AND: update_flags_bin8(res, a, b); break;
			case ALU_SUB: update_flags_sub8(res, a, b); break;
			case ALU_XOR: update_flags_bin8(res, a, b); break;
			case ALU_CMP: update_flags_sub8(res, a, b); break;
		}
	} else {
		switch (func) {
			case ALU_ADD: update_flags_add16(res, a, b); break;
			case ALU_OR:  update_flags_bin16(res, a, b); break;
			case ALU_ADC: update_flags_add16(res, a, b, c); break;
			case ALU_SBB: update_flags_sub16(res, a, b, c); break;
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

inline bool cf8_add(uint16_t res, uint16_t dst, uint16_t src, bool cf) {
	(void)src;
	return readlo(res) < readlo(dst) + readlo(src) + cf;
}

inline bool cf16_add(uint16_t res, uint16_t dst, uint16_t src, bool cf) {
	(void)src;
	return res < dst + src + cf;
}

inline bool cf_w_add(uint16_t res, uint16_t dst, uint16_t src, bool cf, bool w) {
	return !w ? cf8_add(res, dst, src, cf) : cf16_add(res, dst, src, cf);
}

inline bool cf8_sub(uint16_t res, uint16_t dst, uint16_t src, bool cf) {
	(void)res;
	return readlo(dst) < readlo(src) + cf;
}

inline bool cf16_sub(uint16_t res, uint16_t dst, uint16_t src, bool cf) {
	(void)res;
	return dst < src + cf;
}

inline bool cf_w_sub(uint16_t res, uint16_t dst, uint16_t src, bool cf, bool w) {
	return !w ? cf8_sub(res, dst, src, cf) : cf16_sub(res, dst, src, cf);
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

void i8086_t::update_flags_add8(uint8_t res, uint8_t dst, uint8_t src, bool cf) {
	set_cf(cf8_add(res, dst, src, cf));
	set_pf(pf8(res));
	set_af(af8(res, dst, src));
	set_zf(zf8(res));
	set_sf(sf8(res));
	set_of(of8_add(res, dst, src));
}

void i8086_t::update_flags_sub8(uint8_t res, uint8_t dst, uint8_t src, bool cf) {
	set_cf(cf8_sub(res, dst, src, cf));
	set_pf(pf8(res));
	set_af(af8(res, dst, src));
	set_zf(zf8(res));
	set_sf(sf8(res));
	set_of(of8_sub(res, dst, src));
}

void i8086_t::update_flags_bin8(uint8_t res, uint8_t dst, uint8_t src) {
	(void)dst;
	(void)src;
	set_cf(0);
	set_pf(pf8(res));
	set_zf(zf8(res));
	set_sf(sf8(res));
	set_af(0); // TODO: DOSBox does this
	set_of(0);
}

void i8086_t::update_flags_shf8(uint8_t res, uint8_t src, bool cf) {
	set_cf(cf);
	set_pf(pf8(res));
	set_zf(zf8(res));
	set_sf(sf8(res));
	set_of((res & 0x80) != (src & 0x80));
}

void i8086_t::update_flags_rot8(uint8_t res, uint8_t src, bool cf) {
	set_cf(cf);
	set_of((res & 0x80) != (src & 0x80));
}

void i8086_t::update_flags_add16(uint16_t res, uint16_t dst, uint16_t src, bool cf) {
	set_cf(cf16_add(res, dst, src, cf));
	set_pf(pf16(res));
	set_af(af16(res, dst, src));
	set_zf(zf16(res));
	set_sf(sf16(res));
	set_of(of16_add(res, dst, src));
}

void i8086_t::update_flags_sub16(uint16_t res, uint16_t dst, uint16_t src, bool cf) {
	set_cf(cf16_sub(res, dst, src, cf));
	set_pf(pf16(res));
	set_af(af16(res, dst, src));
	set_zf(zf16(res));
	set_sf(sf16(res));
	set_of(of16_sub(res, dst, src));
}

void i8086_t::update_flags_bin16(uint16_t res, uint16_t dst, uint16_t src) {
	(void)dst;
	(void)src;
	set_cf(0);
	set_pf(pf16(res));
	set_zf(zf16(res));
	set_sf(sf16(res));
	set_af(0); // TODO: DOSBox does this
	set_of(0);
}

void i8086_t::update_flags_shf16(uint16_t res, uint16_t src, bool cf) {
	set_cf(cf);
	set_pf(pf16(res));
	set_zf(zf16(res));
	set_sf(sf16(res));
	set_of((res & 0x8000) != (src & 0x8000));
}

void i8086_t::update_flags_rot16(uint16_t res, uint16_t src, bool cf) {
	set_cf(cf);
	set_of((res & 0x8000) != (src & 0x8000));
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
	if (!d) {
		std::swap(a, b);
	}
	uint16_t r = alu_w(func, a, b, w);

	if (func != ALU_CMP) {
		write_modrm(!d ? mem : reg, r);
	}

	cycles += 3;
	// If reading from mem
	if (mem.is_mem) {
		cycles += 6;
	}
	// If writing to mem
	if (!d && func != ALU_CMP) {
		cycles += 7;
	}
}

void i8086_t::op_alu_a_imm() {
	bool w    = op & 1;
	byte func = (op >> 3) & 0b111;

	uint16_t a = read_reg(REG_AX, w);
	uint16_t b = fetch(w);
	uint16_t r = alu_w(func, a, b, w);

	if (func != ALU_CMP) {
		write_reg(REG_AX, r, w);
	}

	cycles += 4;
}

void i8086_t::op_push_sreg() {
	byte sreg = (op >> 3) & 0b111;

	uint16_t v = read_sreg(sreg);
	push(v);

	cycles += 10;
}

void i8086_t::op_pop_sreg() {
	byte sreg = (op >> 3) & 0b111;

	uint16_t v = pop();
	write_sreg(sreg, v);
	int_delay = true;

	cycles += 8;
}

void i8086_t::op_seg_ovr_es() {
	is_prefix = true;
	sreg_ovr = op;
	int_delay = true;

	cycles += 2;
}

void i8086_t::op_daa() {
	uint8_t al = readlo(ax);
	bool    af = !!(flags & FLAG_AF);
	bool    cf = !!(flags & FLAG_CF);
	uint8_t old_al = al;

	if ((al & 0x0f) > 9 || af) {
		al = al + 6;
		af = 1;
	} else {
		af = 0;
	}
	if ((old_al > 0x9f) || cf) {
		al += 0x60;
		cf = 1;
	} else {
		cf = 0;
	}

	writelo(ax, al);

	set_cf(cf);
	set_pf(pf8(al));
	set_af(af);
	set_zf(zf8(al));
	set_sf(sf8(al));

	cycles += 4;
}

void i8086_t::op_seg_ovr_cs() {
	is_prefix = true;
	sreg_ovr = op;
	int_delay = true;

	cycles += 2;
}

void i8086_t::op_das() {
	uint8_t al = readlo(ax);

	uint8_t new_al = al;
	bool    new_af = 0;
	bool    new_cf = 0;

	if ((al & 0x0f) > 9 || get_af()) {
		new_al = al - 6;
		new_cf = flags & FLAG_CF || (new_al > al);
		new_af = 1;
	}
	if ((al > 0x99) || flags & FLAG_CF) {
		new_al -= 0x60;
		new_cf = 1;
	}

	set_cf(new_cf);
	set_pf(pf8(new_al));
	set_af(new_af);
	set_zf(zf8(al));
	set_sf(sf8(new_al));
	writelo(ax, new_al);

	cycles += 4;
}

void i8086_t::op_seg_ovr_ss() {
	is_prefix = true;
	sreg_ovr = op;
	int_delay = true;

	cycles += 2;
}

void i8086_t::op_aaa() {
	uint8_t al = readlo(ax);
	bool    af = get_af();
	bool    cf;

	if ((al & 0x0f) > 9 || af) {
		ax += 0x106;
		af = 1;
		cf = 1;
	} else {
		af = 0;
		cf = 0;
	}
	al &= 0x0f;

	set_cf(cf);
	set_pf(pf8(al));
	set_af(af);
	set_zf(zf8(al));
	set_sf(sf8(al));
	writelo(ax, al);

	cycles += 4;
}

void i8086_t::op_seg_ovr_ds() {
	is_prefix = true;
	sreg_ovr = op;
	int_delay = true;

	cycles += 2;
}

void i8086_t::op_aas() {
	uint8_t al = readlo(ax);
	bool    af = get_af();
	bool    cf;

	if ((al & 0x0f) > 9 || af) {
		ax -= 6;
		writehi(ax, readhi(ax) - 1);
		af = 1;
		cf = 1;
	} else {
		cf = 0;
		af = 0;
	}

	set_cf(cf);
	set_pf(pf8(al));
	set_af(af);
	set_zf(zf8(al));
	set_sf(sf8(al));
	writelo(ax, readlo(ax) & 0x0f);

	cycles += 4;
}

void i8086_t::op_inc_reg() {
	byte reg = op & 0b111;

	uint16_t dst = read_reg(reg, true);
	uint16_t src = 1;
	uint16_t res = dst + 1;
	write_reg(reg, res, true);

	set_pf(pf16(res));
	set_af(af16(res, dst, src));
	set_zf(zf16(res));
	set_sf(sf16(res));
	set_of(of16_add(res, dst, src));

	cycles += 2;
}

void i8086_t::op_dec_reg() {
	byte reg = op & 0b111;

	uint16_t dst = read_reg(reg, true);
	uint16_t src = 1;
	uint16_t res = dst - 1;
	write_reg(reg, res, true);

	set_pf(pf16(res));
	set_af(af16(res, dst, src));
	set_zf(zf16(res));
	set_sf(sf16(res));
	set_of(of16_sub(res, dst, src));

	cycles += 2;
}

void i8086_t::op_push_reg() {
	byte reg = op & 0b111;

	// On 8086 'push ss' pushes the updated value of ss
	// so we can't use ::push(v)
	sp -= 2;
	uint16_t v = read_reg(reg, true);
	mem_write16(ss, sp, v);

	cycles += 11;
}

void i8086_t::op_pop_reg() {
	byte reg = op & 0b111;

	uint16_t v = pop();
	write_reg(reg, v, true);

	cycles += 8;
}

void i8086_t::op_jcc() {
	byte cond = (op >> 1) & 0b111;
	bool neg = op & 1;
	bool r;

	int8_t inc = (int8_t)fetch8();

	switch (cond) {
		case 0b000: r = get_of(); break;
		case 0b001: r = get_cf(); break;
		case 0b010: r = get_zf(); break;
		case 0b011: r = get_cf() | get_zf(); break;
		case 0b100: r = get_sf(); break;
		case 0b101: r = get_pf(); break;
		case 0b110: r = get_sf() ^ get_of(); break;
		case 0b111: r = (get_sf() ^ get_of()) | get_zf(); break;
		default:
			assert(0 && "unreachable");
	}
	if (neg) {
		r = !r;
	}

	if (r) {
		ip += inc;
	}

	cycles += 4;
	if (r) {
		cycles += 12;
	}
}

void i8086_t::op_grp1_rmw_imm() {
	bool w      = !!(op & 1);
	byte modrm  = fetch8();
	byte func   = (modrm >> 3) & 0b111;
	modrm_t mem = modrm_mem_sw(modrm, false, w);
	uint16_t imm;

	switch (op & 3) { // s:w
		case 0b00: imm = fetch8(); break;
		case 0b01: imm = fetch16(); break;
		case 0b10:
		case 0b11: imm = sext(fetch8()); break;
		default:
			assert(0 && "unreachable");
	}

	uint16_t a = read_modrm(mem);
	uint16_t b = imm;
	uint16_t r = alu_w(func, a, b, w);

	if (func != ALU_CMP) {
		write_modrm(mem, r);
	}

	cycles += 4;
	// If reading from mem
	if (mem.is_mem) {
		cycles += 6;
	}
	// If writing to mem
	if (func != ALU_CMP) {
		cycles += 7;
	}
}

void i8086_t::op_test_rm_r() {
	bool w     = !!(op & 0b01);
	byte modrm = fetch8();

	modrm_t mem = modrm_mem_sw(modrm, false, w);
	modrm_t reg = modrm_reg_sw(modrm, false, w);

	uint16_t a = read_modrm(mem);
	uint16_t b = read_modrm(reg);

	(void)alu_w(ALU_AND, a, b, w);
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

	cycles += 2;
	if (mem.is_mem) {
		// If one argument is mem
		cycles += 6;
		if (!d) {
			// If destination is mem
			cycles += 1;
		}
	}
}

void i8086_t::op_mov_rm16_sreg() {
	bool d = !!(op & 2);

	byte modrm = fetch8();
	byte sreg = (modrm >> 3) & 0b111;

	if (!d) {
		modrm_t dst = modrm_mem_sw(modrm, false, true);
		write_modrm(dst, read_sreg(sreg));

		cycles += 2;
		if (dst.is_mem) {
			cycles += 7;
		}
	} else {
		modrm_t src = modrm_mem_sw(modrm, false, true);
		write_sreg(sreg, read_modrm(src));
		int_delay = true;

		cycles += 2;
		if (src.is_mem) {
			cycles += 6;
		}
	}
}

void i8086_t::op_lea_r16_m16() {
	byte modrm = fetch8();
	modrm_t src = modrm_mem_sw(modrm, false, true);
	modrm_t dst = modrm_reg_sw(modrm, false, true);
	write_modrm(dst, src.ofs);

	cycles += 2;
}

void i8086_t::op_pop_rm16() {
	byte modrm = fetch8();

	modrm_t dst = modrm_mem_sw(modrm, false, true);
	write_modrm(dst, pop());

	cycles += 8;
	if (dst.is_mem) {
		cycles += 11;
	}
}

void i8086_t::op_xchg_ax_r() {
	byte reg = op & 0b111;

	cycles += 3;

	uint16_t tmp = read_reg(reg, true);
	write_reg(reg, ax, true);
	ax = tmp;
}

void i8086_t::op_cbw() {
	ax = sext(readlo(ax));

	cycles += 2;
}

void i8086_t::op_cwd() {
	dx = (ax & 0x8000) ? 0xffff : 0x0000;

	cycles += 5;
}

void i8086_t::op_call_far() {
	uint16_t ofs = fetch16();
	uint16_t seg = fetch16();

	push(cs);
	push(ip);

	call_stack.push_back({
		{cs, op_ip},
		{seg, ofs},
		false
	});

	cs = seg;
	ip = ofs;

	cycles += 28;
}

void i8086_t::op_wait() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_pushf() {
	push(flags);
}

void i8086_t::op_popf() {
	flags = pop();

	cycles += 8;
}

void i8086_t::op_sahf() {
	flags = readhi(ax);

	cycles += 4;
}

void i8086_t::op_lahf() {
	writehi(ax, flags);

	cycles += 4;
}

void i8086_t::op_mov_a_m() {
	bool w = !!(op & 1);
	bool d = !!(op & 2);
	uint16_t seg = read_sreg_ovr(SEG_DS);
	uint16_t ofs = fetch16();
	uint16_t v;

	if (!d) {
		v = mem_read(seg, ofs, w);
		write_reg(REG_AX, v, w);
	} else {
		v = read_reg(REG_AX, w);
		mem_write(seg, ofs, v, w);
	}

	cycles += 10;
}

void i8086_t::op_movs() {
	bool     w        = !!(op & 1);
	int      delta    = strop_delta(w);
	byte     src_sreg = get_sreg_ovr(SEG_DS);
	uint16_t src_seg  = read_sreg(src_sreg);

	cycles += 1;

	if (repmode == REP_NONE) {
		cycles += 8;
		goto inst;
	}
repeat:
	if (cx == 0) {
		return;
	}

	// TODO: Service interrupts
	--cx;
inst:
	mem_write(es, di, mem_read(src_seg, si, w), w);

	si += delta;
	di += delta;

	cycles += 17;

	if (repmode != REP_NONE) {
		goto repeat;
	}
}

void i8086_t::op_cmps() {
	bool     w      = !!(op & 1);
	int      delta  = strop_delta(w);
	uint16_t si_seg = read_sreg_ovr(SEG_DS);
	uint16_t di_seg = read_sreg(SEG_ES);

	if (repmode == REP_NONE) {
		cycles += 9;
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
	uint16_t a = mem_read(di_seg, di, w);
	uint16_t b = mem_read(si_seg, si, w);
	si += delta;
	di += delta;

	alu_w(ALU_CMP, b, a, w);

	cycles += 22;

	if (repmode == REP_REP && get_zf()) {
		goto repeat;
	}
	if (repmode == REP_REPNE && !get_zf()) {
		goto repeat;
	}
}

void i8086_t::op_test_a_imm() {
	bool w = op & 1;
	uint16_t imm = fetch(w);

	uint16_t a = read_reg(REG_AX, w);
	alu_w(ALU_AND, a, imm, w);
}

void i8086_t::op_stos() {
	bool     w     = op & 1;
	uint16_t v     = read_reg(REG_AX, w);
	int      delta = strop_delta(w);

	cycles += 1;
	if (repmode != REP_NONE) {
		cycles += 9;
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
	mem_write(read_sreg(SEG_ES), di, v, w);
	di += delta;

	cycles += 10;

	if (repmode != REP_NONE) {
		goto repeat;
	}
}

void i8086_t::op_lods() {
	bool     w     = op & 1;
	uint16_t seg   = read_sreg_ovr(SEG_DS);
	int      delta = strop_delta(w);
	uint16_t v;

	if (repmode != REP_NONE) {
		cycles += 9;
	}

	if (repmode == REP_NONE) {
		goto inst;
	}
repeat:
	if (cx == 0) {
		return;
	}
	cycles += 1;

	// TODO: Service interrupts
	--cx;
inst:
	v = mem_read(seg, si, w);
	write_reg(REG_AX, v, w);
	si += delta;

	cycles += 12;

	if (repmode != REP_NONE) {
		goto repeat;
	}
}

void i8086_t::op_scas() {
	bool     w      = !!(op & 1);
	int      delta  = strop_delta(w);
	uint16_t di_seg = read_sreg_ovr(SEG_ES);

	uint16_t a = read_reg(REG_AX, w);
	uint16_t b;

	if (repmode != REP_NONE) {
		cycles += 9;
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
	b = mem_read(di_seg, di, w);
	di += delta;

	alu_w(ALU_CMP, a, b, w);

	cycles += 15;

	if (repmode == REP_REP && get_zf()) {
		goto repeat;
	}
	if (repmode == REP_REPNE && !get_zf()) {
		goto repeat;
	}
}

void i8086_t::op_mov_reg_imm() {
	byte reg = op & 0b111;
	bool w   = (op >> 3) & 1;
	uint16_t imm = !w ? fetch8() : fetch16();
	write_reg(reg, imm, w);

	cycles += 4;
}

void i8086_t::op_ret_imm16_intraseg() {
	ip = pop();
	sp += fetch16();

	if (!call_stack.empty()) {
		call_stack.pop_back();
	}
}

void i8086_t::op_ret_intraseg() {
	ip = pop();

	if (!call_stack.empty()) {
		call_stack.pop_back();
	}
}

void i8086_t::op_les_r16_m16() {
	byte modrm = fetch8();
	modrm_t src = modrm_mem_sw(modrm, false, true);
	modrm_t dst = modrm_reg_sw(modrm, false, true);

	assert(src.is_mem);

	uint16_t ofs = read_modrm(src);
	uint16_t seg = read_modrm(src + 2);

	write_modrm(dst, ofs);
	es = seg;

	cycles += 16;
}

void i8086_t::op_lds_r16_m16() {
	byte modrm = fetch8();
	modrm_t src = modrm_mem_sw(modrm, false, true);
	modrm_t dst = modrm_reg_sw(modrm, false, true);

	assert(src.is_mem);

	uint16_t ofs = read_modrm(src);
	uint16_t seg = read_modrm(src + 2);

	write_modrm(dst, ofs);
	ds = seg;

	cycles += 16;
}

void i8086_t::op_mov_m_imm() {
	bool w = op & 1;
	byte modrm = fetch8();
	modrm_t dst = modrm_mem_sw(modrm, false, w);
	uint16_t imm = fetch(w);

	cycles += 10;

	write_modrm(dst, imm);
}

void i8086_t::op_ret_imm16_interseg() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_ret_interseg() {
	ip = pop();
	cs = pop();

	if (!call_stack.empty()) call_stack.pop_back();
}

void i8086_t::op_int_3() {
	call_int(3);

	cycles += 52;
}

void i8086_t::op_int_imm8() {
	byte imm = fetch8();
	call_int(imm);

	cycles += 51; // TODO: 51 or 52 if imm=3 ?
}

void i8086_t::op_into() {
	cycles += 4;
	if (get_of()) {
		call_int(4);
		cycles += 49;
	}
}

void i8086_t::op_iret() {
	ip = pop();
	cs = pop();
	flags = pop();
	int_delay = true;

	cycles += 24;

	call_stack.pop_back();
}

// TODO: Rotates also need to update OF
inline void rolb(uint16_t &v, bool &cf, byte n = 1) {
	while (n--) {
		cf = !!(v & 0x80);
		v = (v << 1) | ((v & 0x80) ? 0x01 : 0x00);
	}
}

inline void rorb(uint16_t &v, bool &cf, byte n = 1) {
	while (n--) {
		cf = !!(v & 0x01);
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

inline void rolw(uint16_t &v, bool &cf, byte n = 1) {
	while (n--) {
		cf = !!(v & 0x8000);
		v = (v << 1) | ((v & 0x8000) ? 0x01 : 0x00);
	}
}

inline void rorw(uint16_t &v, bool &cf, byte n = 1) {
	while (n--) {
		cf = v & 1;
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
		cf = !!(v & 0x8000);
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

	byte n = !v ? 1 : readlo(cx);

	// TODO: DOSBox doesn't update flags if n is zero
	if (!n) {
		return;
	}

	if (!v) {
		cycles += 2;
		if (dst.is_mem) {
			cycles += 13;
		}
	} else {
		cycles += 8 + 4*n;
		if (dst.is_mem) {
			cycles += 12;
		}
	}

	uint16_t src = read_modrm(dst);
	uint16_t res = src;
	bool cf = flags & FLAG_CF;
	if (!w) {
		switch (func) {
			case 0: rolb(res, cf, n); break;
			case 1: rorb(res, cf, n); break;
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
			case 0: rolw(res, cf, n); break;
			case 1: rorw(res, cf, n); break;
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
}

void i8086_t::op_aam() {
	uint8_t imm = fetch8();
	uint8_t tmp_al = readlo(ax);
	writehi(ax, tmp_al / imm);
	writelo(ax, tmp_al % imm);

	uint8_t al = readlo(ax);
	set_pf(pf8(al));
	set_zf(zf8(al));
	set_sf(sf8(al));

	cycles += 83;
}

void i8086_t::op_aad() {
	uint8_t imm = fetch8();
	uint8_t tmp_al = readlo(ax);
	uint8_t tmp_ah = readhi(ax);

	uint8_t al = (tmp_al + (tmp_ah * imm)) & 0xff;

	writehi(ax, 0);
	writelo(ax, al);

	set_pf(pf8(al));
	set_zf(zf8(al));
	set_sf(sf8(al));

	cycles += 60;
}

void i8086_t::op_xlat() {
	byte v = mem_read8(read_sreg_ovr(SEG_DS), bx + readlo(ax));
	writelo(ax, v);

	cycles += 11;
}

void i8086_t::op_esc() {
	unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_loopnz() {
	int8_t inc = (int8_t)fetch8();

	bool cond = --cx != 0 && !get_zf();
	if (cond) {
		ip += inc;
	}

	cycles += 5;
	if (cond) {
		cycles += 14;
	}
}

void i8086_t::op_loopz() {
	int8_t inc = (int8_t)fetch8();

	bool cond = --cx != 0 && get_zf();
	if (cond) {
		ip += inc;
	}

	cycles += 6;
	if (cond) {
		cycles += 12;
	}
}

void i8086_t::op_loop() {
	int8_t inc = (int8_t)fetch8();

	bool cond = --cx != 0;
	if (cond) {
		ip += inc;
	}

	cycles += 5;
	if (cond) {
		cycles += 19;
	}
}

void i8086_t::op_jcxz() {
	int8_t inc = (int8_t)fetch8();
	if (!cx) {
		ip += inc;
	}

	cycles += 6;
	if (!cx) {
		cycles += 12;
	}
}

void i8086_t::op_in_al_imm8() {
	byte port = fetch8();
	byte v;

	v = read(IO, port, W8);
	writelo(ax, v);

	cycles += 10;
}

void i8086_t::op_in_ax_imm8() {
	byte port = fetch8();

	uint16_t v = read(IO, port, W16);
	ax = v;

	cycles += 10;
}

void i8086_t::op_out_al_imm8() {
	byte port = fetch8();

	write(IO, port, W8, readlo(ax));

	cycles += 10;
}

void i8086_t::op_out_ax_imm8() {
	uint16_t imm = fetch16();

	write(IO, imm, W16, ax);

	cycles += 8;
}

void i8086_t::op_call_near() {
	uint16_t inc = fetch16();

	sp -= 2;
	mem_write16(ss, sp, ip);

	call_stack.push_back({
		{cs, op_ip},
		{cs, uint16_t(ip + inc)},
		false
	});

	ip += inc;

	cycles += 19;
}

void i8086_t::op_jmp_near() {
	uint16_t inc = fetch16();

	ip += inc;
}

void i8086_t::op_jmp_far() {
	uint16_t ofs = fetch16();
	uint16_t seg = fetch16();

	cs = seg;
	ip = ofs;
}

void i8086_t::op_jmp_short() {
	uint16_t inc = sext(fetch8());

	ip += inc;
}

void i8086_t::op_in_al_dx() {
	uint16_t port = dx;
	uint16_t v;

	v = read(IO, port, W8);
	writelo(ax, v);

	cycles += 8;
}

void i8086_t::op_in_ax_dx() {
	uint16_t port = dx;
	uint16_t v = read(IO, port, W16);
	ax = v;

	cycles += 8;
}

void i8086_t::op_out_al_dx() {
	uint16_t port = dx;

	write(IO, port, W8, readlo(ax));
}

void i8086_t::op_out_ax_dx() {
	uint16_t port = dx;

	write(IO, port, W16, ax);
}

void i8086_t::op_lock_prefix() {
	// unimplemented(__FUNCTION__, __LINE__);
	int_delay = true;
	cycles += 2;
}

void i8086_t::op_repne() {
	is_prefix = true;
	repmode = REP_REPNE;
	int_delay = true;
}

void i8086_t::op_rep() {
	is_prefix = true;
	repmode = REP_REP;
	int_delay = true;
}

void i8086_t::op_hlt() {
	cycles += 2;
	// unimplemented(__FUNCTION__, __LINE__);
}

void i8086_t::op_cmc() {
	set_cf(!get_cf());

	cycles += 2;
}

static inline
int8_t utos8(uint8_t v) {
	if (v <= INT8_MAX) {
		return static_cast<int8_t>(v);
	}
	return static_cast<int8_t>(v - INT8_MIN) + INT8_MIN;
}

static inline
int16_t utos16(uint16_t v) {
	if (v <= INT16_MAX) {
		return static_cast<int16_t>(v);
	}
	return static_cast<int16_t>(v - INT16_MIN) + INT16_MIN;
}

static inline
int32_t utos32(uint32_t v) {
	if (v <= INT32_MAX) {
		return static_cast<int32_t>(v);
	}
	return static_cast<int32_t>(v - INT32_MIN) + INT32_MIN;
}

void i8086_t::op_grp3_rmw() {
	bool w = !!(op & 1);
	byte modrm  = fetch8();
	byte func   = (modrm >> 3) & 0b111;
	modrm_t mem = modrm_mem_sw(modrm, false, w);
	uint16_t res, src;

	// TODO: Clean this up, move cases to functions.
	switch (func) {
		case 0b000: // test
		case 0b001: {
			uint16_t a = read_modrm(mem);
			uint16_t b = fetch(w);
			alu_w(ALU_AND, a, b, w);
			break;
		}
		case 0b010: // not
			src = read_modrm(mem);
			res = ~src;
			write_modrm(mem, res);
			break;
		case 0b011: // neg
			src = read_modrm(mem);
			res = alu_w(ALU_SUB, 0, src, w);
			write_modrm(mem, res);

			cycles += 3;
			if (mem.is_mem) {
				cycles += 13;
			}
			break;
		case 0b100: // mul
			src = read_modrm(mem);
			if (!w) {
				ax = readlo(ax) * src;
				set_of(readhi(ax) != 0);
				set_cf(readhi(ax) != 0);
			} else {
				uint32_t tmp = (uint32_t)ax * (uint32_t)src;
				dx = tmp >> 16;
				ax = tmp & 0xffff;
				set_of(dx != 0);
				set_cf(dx != 0);
			}
			set_zf(ax == 0); // TODO: DOSBox defines zf, spec says undefined.
			break;
		case 0b101: // imul
			src = read_modrm(mem);
			if (!w) {
				uint16_t tmp_xp = int16_t(utos8(readlo(ax))) * int16_t(utos8(src));
				ax = tmp_xp;

				if (sext(readlo(tmp_xp)) == tmp_xp) {
					set_cf(false);
					set_of(false);
				} else {
					set_cf(true);
					set_of(true);
				}
			} else {
				int32_t tmp_xp = int32_t(utos16(ax)) * int32_t(utos16(src));
				ax = ((tmp_xp >>  0) & 0xffff);
				dx = ((tmp_xp >> 16) & 0xffff);
				if (sext16(ax) == tmp_xp) {
					set_cf(false);
					set_of(false);
				} else {
					set_cf(true);
					set_of(true);
				}
			}
			break;
		case 0b110: // div
			src = read_modrm(mem);
			if (src == 0) {
				call_int(0);
				return;
			}

			if (!w) {
				uint16_t quotient  = ax / src;
				uint16_t remainder = ax % src;

				if (quotient > 0xff) {
					call_int(0);
					return;
				}

				writelo(ax, quotient);
				writehi(ax, remainder);
			} else {
				uint32_t dividend  = (((uint32_t)dx) << 16) + (uint32_t)ax;
				uint32_t quotient  = dividend / src;
				uint32_t remainder = dividend % src;

				if (quotient > 0xffff) {
					call_int(0);
					return;
				}

				ax = quotient;
				dx = remainder;
			}
			break;
		case 0b111: // idiv
			src = read_modrm(mem);
			if (src == 0) {
				call_int(0);
				return;
			}

			if (!w) {
				int16_t dividend = utos16(ax);
				int8_t  divisor  = utos8(src);

				if (dividend == 0x7fff && divisor == -1) {
					call_int(0);
					return;
				}

				int16_t quotient  = dividend / divisor;
				int16_t remainder = dividend % divisor;

				if (quotient > INT8_MAX || quotient < INT8_MIN) {
					call_int(0);
					return;
				}

				writelo(ax, quotient);
				writehi(ax, remainder);
			} else {
				int32_t dividend = utos32((((uint32_t)dx) << 16) + (uint32_t)ax);
				int16_t divisor  = utos16(src);

				if (dividend == INT32_MAX && divisor == -1) {
					call_int(0);
					return;
				}

				int32_t quotient  = dividend / divisor;
				int32_t remainder = dividend % divisor;

				if (quotient > INT16_MAX || quotient < INT16_MIN) {
					call_int(0);
					return;
				}

				ax = quotient;
				dx = remainder;
			}
			break;
	}
}

void i8086_t::op_clc() {
	set_cf(false);

	cycles += 2;
}

void i8086_t::op_stc() {
	set_cf(true);

	cycles += 2;
}

void i8086_t::op_cli() {
	set_if(false);

	cycles += 2;
}

void i8086_t::op_sti() {
	set_if(true);
	int_delay = true;

	cycles += 2;
}

void i8086_t::op_cld() {
	set_df(false);

	cycles += 2;
}

void i8086_t::op_std() {
	set_df(true);

	cycles += 2;
}

void i8086_t::op_grp4_rm8() {
	byte modrm = fetch8();
	byte subop = (modrm >> 3) & 0b111;
	uint16_t src, dst, res;
	modrm_t mem;

	if (modrm == 0x38) {
		uint16_t callback_id = fetch16();
		callback_t callback = callbacks[callback_id];
		callback();
		return;
	}

	switch (subop) {
		case 0b000: // inc
			mem = modrm_mem_sw(modrm, false, false);
			dst = read_modrm(mem);
			src = 1;
			res = dst + src;
			write_modrm(mem, res);

			set_pf(pf8(res));
			set_af(af8(res, dst, src));
			set_zf(zf8(res));
			set_sf(sf8(res));
			set_of(of8_add(res, dst, src));

			cycles += 3;
			if (mem.is_mem) {
				cycles += 12;
			}
			break;
		case 0b001: // dec
			mem = modrm_mem_sw(modrm, false, false);
			dst = read_modrm(mem);
			src = 1;
			res = dst - src;
			write_modrm(mem, res);

			set_pf(pf8(res));
			set_af(af8(res, dst, src));
			set_zf(zf8(res));
			set_sf(sf8(res));
			set_of(of8_sub(res, dst, src));

			cycles += 3;
			if (mem.is_mem) {
				cycles += 12;
			}
			break;
	}
}

void i8086_t::op_grp5() {
	byte modrm = fetch8();
	byte subop = (modrm >> 3) & 0b111;
	modrm_t mem = modrm_mem_sw(modrm, false, true);
	switch (subop) {
		case 0b000: {
			uint16_t v = read_modrm(mem);
			uint16_t res = alu_w(ALU_ADD, v, 1, true);
			write_modrm(mem, res);
			break;
		}
		case 0b001: {
			uint16_t v = read_modrm(mem);
			uint16_t res = alu_w(ALU_SUB, v, 1, true);
			write_modrm(mem, res);
			break;
		}
		case 0b010: {
			uint16_t ofs = read_modrm(mem);
			push(ip);
			ip = ofs;

			cycles += 16;
			if (mem.is_mem) {
				cycles += 5;
			}

			call_stack.push_back({
				{cs, op_ip},
				{cs, ofs},
				false
			});
			break;
		}
		case 0b011: {
			assert(mem.is_mem && "TODO: Illegal opcode");
			uint16_t ofs = read_modrm(mem);
			mem.ofs += 2;
			uint16_t seg = read_modrm(mem);

			push(cs);
			push(ip);

			cycles += 37;

			call_stack.push_back({
				{cs, op_ip},
				{seg, ofs},
				false
			});

			cs = seg;
			ip = ofs;
			break;
		}
		case 0b100: {
			uint16_t ofs = read_modrm(mem);
			ip = ofs;
			break;
		}
		case 0b110: { // push_rm
			uint16_t v = read_modrm(mem);
			push(v);

			cycles += 11;
			if (mem.is_mem) {
				cycles += 7;
			}
			break;
		}
		default:
			printf("op_grp5 subop %x unimplemented\n", subop);
			unimplemented(__FUNCTION__, __LINE__);
	}
}
