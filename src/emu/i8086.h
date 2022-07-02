#ifndef EMU_I8086
#define EMU_I8086

#include "emu/emu.h"
#include "emu/cpu_device.h"
#include "support/types.h"

#include <functional>
#include <vector>

class ibm5160_t;

struct i8086_addr_t {
	uint16_t seg;
	uint16_t ofs;
};

class i8086_t : public cpu_device_t {
	int instr_count = 0;
	uint64_t cycles = 0;

	std::vector<callback_t>       callbacks;
	i8086_addr_t                  callback_base_addr;
	i8086_addr_t                  callback_next_addr;

public:
	read_cb_t  read;
	write_cb_t write;

	i8086_t();

	double frequency_in_mhz() {
		return 5.0;
	}

	uint64_t next_cycles();
	uint64_t run_cycles(uint64_t cycles);

	void dump_state();
	void log_state();

	void         set_callback_base(uint16_t callback_base_seg);
	i8086_addr_t install_callback(uint16_t seg, uint16_t ofs, callback_t callback);

	uint32_t step();
	uint32_t dispatch();

	bool int_delay;
	bool int_nmi;
	bool int_intr;
	byte int_number;

	void raise_nmi();
	void raise_intr(byte num);
	void call_int(byte num);

	/* Registers */
	enum {
		REG_AX,
		REG_CX,
		REG_DX,
		REG_BX,
	};

	enum {
		SEG_ES,
		SEG_CS,
		SEG_SS,
		SEG_DS,
	};

	enum {
		ALU_ADD,
		ALU_OR,
		ALU_ADC,
		ALU_SBB,
		ALU_AND,
		ALU_SUB,
		ALU_XOR,
		ALU_CMP,
		ALU_INC,
	};

	byte op;
	bool is_prefix;
	byte sreg_ovr;

	enum {
		REP_NONE,
		REP_REPNE,
		REP_REP,
	} repmode;

	uint16_t log_cs;
	uint16_t log_ip;

	// Registers
	uint16_t ip;
	uint16_t op_ip;

	uint16_t es;
	uint16_t cs;
	uint16_t ss;
	uint16_t ds;

	uint16_t ax;
	uint16_t cx;
	uint16_t dx;
	uint16_t bx;

	uint16_t sp;
	uint16_t bp;
	uint16_t di;
	uint16_t si;

	uint16_t flags;

	enum {
		FLAG_CF = (1 <<  0),
		FLAG_PF = (1 <<  2),
		FLAG_AF = (1 <<  4),
		FLAG_ZF = (1 <<  6),
		FLAG_SF = (1 <<  7),
		FLAG_TF = (1 <<  8),
		FLAG_IF = (1 <<  9),
		FLAG_DF = (1 << 10),
		FLAG_OF = (1 << 11),
	};

	void set_flags(uint16_t mask, bool cond) {
		flags = cond ? (flags | mask) : (flags & ~mask);
	}

	int get_log_flags() { return flags & (FLAG_CF | FLAG_PF | FLAG_ZF | FLAG_SF | FLAG_TF | FLAG_IF | FLAG_DF | FLAG_OF); }

	bool get_cf() { return !!(flags & FLAG_CF); }
	bool get_pf() { return !!(flags & FLAG_PF); }
	bool get_af() { return !!(flags & FLAG_AF); }
	bool get_zf() { return !!(flags & FLAG_ZF); }
	bool get_sf() { return !!(flags & FLAG_SF); }
	bool get_tf() { return !!(flags & FLAG_TF); }
	bool get_if() { return !!(flags & FLAG_IF); }
	bool get_df() { return !!(flags & FLAG_DF); }
	bool get_of() { return !!(flags & FLAG_OF); }

	void set_cf(bool cond) { set_flags(FLAG_CF, cond); }
	void set_pf(bool cond) { set_flags(FLAG_PF, cond); }
	void set_af(bool cond) { set_flags(FLAG_AF, cond); }
	void set_zf(bool cond) { set_flags(FLAG_ZF, cond); }
	void set_sf(bool cond) { set_flags(FLAG_SF, cond); }
	void set_tf(bool cond) { set_flags(FLAG_TF, cond); }
	void set_if(bool cond) { set_flags(FLAG_IF, cond); }
	void set_df(bool cond) { set_flags(FLAG_DF, cond); }
	void set_of(bool cond) { set_flags(FLAG_OF, cond); }

	int get_instr_count() { return instr_count; }

	struct modrm_t {
		byte     v;
		byte     is_mem : 1;
		byte     w : 1;
		byte     s : 1;
		union {
			byte     reg;
			struct {
				byte sreg;
				uint16_t ofs;
			};
		};

		void print();

		modrm_t operator+(int d) {
			assert(is_mem);
			modrm_t modrm = *this;
			modrm.ofs += d;
			return modrm;
		}
	};

	modrm_t modrm_mem_sw(byte modrm, bool s, bool w);
	modrm_t modrm_reg_sw(byte modrm, bool s, bool w);

	uint16_t read_modrm(modrm_t modrm);
	void     write_modrm(modrm_t modrm, uint16_t v);

	static const char *str_reg(byte reg, bool w);
	uint16_t read_reg(byte reg, bool w);
	void     write_reg(byte reg, uint16_t v, bool w);

// protected:
	byte     mem_read8(uint16_t seg, uint16_t ofs);
	uint16_t mem_read16(uint16_t seg, uint16_t ofs);
	uint16_t mem_read(uint16_t seg, uint16_t ofs, bool w) {
		return !w ? mem_read8(seg, ofs) : mem_read16(seg, ofs);
	}

	void mem_write8(uint16_t seg, uint16_t ofs, byte v);
	void mem_write16(uint16_t seg, uint16_t ofs, uint16_t v);
	void mem_write(uint16_t seg, uint16_t ofs, uint16_t v, bool w) {
		if (!w) {
			mem_write8(seg, ofs, v);
		} else {
			mem_write16(seg, ofs, v);
		}
	}

	byte     fetch8();
	uint16_t fetch16();
	uint16_t fetch(bool w) {
		return !w ? fetch8() : fetch16();
	}

	void     push(uint16_t v);
	uint16_t pop();

	static const char *str_imm(uint16_t imm);
	static const char *str_sreg(byte sreg);

	uint16_t           read_sreg(byte sreg);
	void               write_sreg(byte sreg, uint16_t v);
	byte               get_sreg_ovr(byte sreg_def);
	uint16_t           read_sreg_ovr(byte sreg_def);
	void               write_sreg_ovr(byte sreg_def, uint16_t v);

	const char *str_w(bool w);

	int16_t strop_delta(bool w) {
		if (!w) {
			if (!(flags & FLAG_DF)) {
				return 1;
			} else {
				return -1;
			}
		} else {
			if (!(flags & FLAG_DF)) {
				return 2;
			} else {
				return -2;
			}
		}
	}

	uint16_t alu_w(byte func, uint16_t a, uint16_t b, bool w);

	void update_flags_add8(uint8_t res, uint8_t dst, uint8_t src, bool cf = 0);
	void update_flags_sub8(uint8_t res, uint8_t dst, uint8_t src, bool cf = 0);
	void update_flags_bin8(uint8_t res, uint8_t dst, uint8_t src);
	void update_flags_shf8(uint8_t res, uint8_t src, bool cf);
	void update_flags_rot8(uint8_t res, uint8_t src, bool cf);
	void update_flags_add16(uint16_t res, uint16_t dst, uint16_t src, bool cf = 0);
	void update_flags_sub16(uint16_t res, uint16_t dst, uint16_t src, bool cf = 0);
	void update_flags_bin16(uint16_t res, uint16_t dst, uint16_t src);
	void update_flags_shf16(uint16_t res, uint16_t src, bool cf);
	void update_flags_rot16(uint16_t res, uint16_t src, bool cf);

	void unimplemented(const char *op_name, int line);
	void op_unused();

	void op_alu_r_rm();
	void op_alu_a_imm();
	void op_push_sreg();
	void op_pop_sreg();
	void op_seg_ovr_es();
	void op_daa();
	void op_seg_ovr_cs();
	void op_das();
	void op_seg_ovr_ss();
	void op_aaa();
	void op_seg_ovr_ds();
	void op_aas();
	void op_inc_reg();
	void op_dec_reg();
	void op_push_reg();
	void op_pop_reg();
	void op_jcc();
	void op_grp1_rmw_imm();
	void op_test_rm_r();
	void op_xchg_rm_r();
	void op_mov_rm_r();
	void op_mov_rm16_sreg();
	void op_lea_r16_m16();
	void op_pop_rm16();
	void op_xchg_ax_r();
	void op_cbw();
	void op_cwd();
	void op_call_far();
	void op_wait();
	void op_pushf();
	void op_popf();
	void op_sahf();
	void op_lahf();
	void op_mov_a_m();
	void op_movs();
	void op_cmps();
	void op_test_a_imm();
	void op_stos();
	void op_lods();
	void op_scas();
	void op_mov_reg_imm();
	void op_ret_imm16_intraseg();
	void op_ret_intraseg();
	void op_les_r16_m16();
	void op_lds_r16_m16();
	void op_mov_m_imm();
	void op_ret_imm16_interseg();
	void op_ret_interseg();
	void op_int_3();
	void op_int_imm8();
	void op_into();
	void op_iret();
	void op_grp2_rmw();
	void op_aam();
	void op_aad();
	void op_xlat();
	void op_esc();
	void op_loopnz();
	void op_loopz();
	void op_loop();
	void op_jcxz();
	void op_in_al_imm8();
	void op_in_ax_imm8();
	void op_out_al_imm8();
	void op_out_ax_imm8();
	void op_call_near();
	void op_jmp_near();
	void op_jmp_far();
	void op_jmp_short();
	void op_in_al_dx();
	void op_in_ax_dx();
	void op_out_al_dx();
	void op_out_ax_dx();
	void op_lock_prefix();
	void op_repne();
	void op_rep();
	void op_hlt();
	void op_cmc();
	void op_grp3_rmw();
	void op_clc();
	void op_stc();
	void op_cli();
	void op_sti();
	void op_cld();
	void op_std();
	void op_grp4_rm8();
	void op_grp5();
};

#endif
