#ifndef EMU_I80806
#define EMU_I80806

#include "support/types.h"

class ibm5160_t;

class i8086_t {
	ibm5160_t *machine;
	byte *memory;
	int instr_count = 0;
	uint64_t cycles = 0;

public:
	i8086_t();
	void set_machine(ibm5160_t *machine);
	void set_memory(byte *memory);
	void dump_state();
	void log_state();
	uint32_t step();
	uint32_t dispatch();

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

	struct {
		uint16_t u15 : 1;
		uint16_t u14 : 1;
		uint16_t u13 : 1;
		uint16_t u12 : 1;
		uint16_t of  : 1;
		uint16_t df  : 1;
		uint16_t tf  : 1;
		uint16_t if_ : 1;

		uint16_t sf  : 1;
		uint16_t zf  : 1;
		uint16_t u5  : 1;
		uint16_t af  : 1;
		uint16_t u3  : 1;
		uint16_t pf  : 1;
		uint16_t u1  : 1;
		uint16_t cf  : 1;
	} flags;

	struct modrm_t {
		byte     v;
		byte     is_mem : 1;
		byte     w : 1;
		byte     s : 1;
		union {
			byte     reg;
			struct {
				byte sreg_ovr;
				uint16_t ofs;
			};
		};

		void print();
	};

	modrm_t modrm_mem_sw(byte modrm, bool s, bool w);
	modrm_t modrm_reg_sw(byte modrm, bool s, bool w);

	uint16_t read_modrm(modrm_t modrm, byte sreg_def = SEG_DS);
	void     write_modrm(modrm_t modrm, uint16_t v, byte sreg_def = SEG_DS);

	static const char *str_reg(byte reg, bool w);
	uint16_t read_reg(byte reg, bool w);
	void     write_reg(byte reg, uint16_t v, bool w);

// protected:
	byte     read_mem8(uint16_t seg, uint16_t ofs);
	uint16_t read_mem16(uint16_t seg, uint16_t ofs);
	uint16_t read_mem(uint16_t seg, uint16_t ofs, bool w) {
		return !w ? read_mem8(seg, ofs) : read_mem16(seg, ofs);
	}

	void write_mem8(uint16_t seg, uint16_t ofs, byte v);
	void write_mem16(uint16_t seg, uint16_t ofs, uint16_t v);
	void write_mem(uint16_t seg, uint16_t ofs, uint16_t v, bool w) {
		if (!w) {
			write_mem8(seg, ofs, v);
		} else {
			write_mem16(seg, ofs, v);
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
			if (!flags.df) {
				return 1;
			} else {
				return -1;
			}
		} else {
			if (!flags.df) {
				return 2;
			} else {
				return -2;
			}
		}
	}

	uint16_t alu_sw(byte func, uint16_t a, uint16_t b, bool w);

	void update_flags_add8(uint8_t res, uint8_t dst, uint8_t src);
	void update_flags_sub8(uint8_t res, uint8_t dst, uint8_t src);
	void update_flags_bin8(uint8_t res, uint8_t dst, uint8_t src);
	void update_flags_shf8(uint8_t res, uint8_t src, bool cf);
	void update_flags_rot8(uint8_t res, uint8_t src, bool cf);
	void update_flags_add16(uint16_t res, uint16_t dst, uint16_t src);
	void update_flags_sub16(uint16_t res, uint16_t dst, uint16_t src);
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
	void op_test_rm8_r8();
	void op_test_rm16_r16();
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
