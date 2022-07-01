#include "dos.h"

#include "dos/dos_environment.h"
#include "emu/i8086.h"
#include "emu/ibm5160.h"

#include <cstring>

void dos_t::install() {
	cpu = (i8086_t *)machine->cpu;

	cpu->install_callback(0x0000, 4 * 0x21, std::bind(&dos_t::int21, this));
	cpu->install_callback(0x0000, 4 * 0x33, std::bind(&dos_t::int33, this));

	mcb_t initial_mcb = mcb_t::build_mcb(machine, 0x0158);
	initial_mcb.set_size_in_paras(0x9fff - 0x0158);
	initial_mcb.set_owner_pid(0);
	initial_mcb.set_is_last();

	// Allocate a block for the default environment
	uint16_t mcb_size = 10; // 10 paragraphs, 160 bytes.
	mcb_t env_mcb = initial_mcb;
	env_mcb.split(mcb_size);
	env_mcb.set_owner_pid(8);
	uint16_t env_seg = env_mcb.data_seg();

	// Set up the default environment
	dos_environment_t env = dos_environment_t::create_in_seg(machine, env_seg, mcb_size);

	set_in_env(env_seg, "COMSPEC=C:\\COMMAND.COM");
	set_in_env(env_seg, "PATH=C:\\DOS");

	if (!validate_mcb_chain()) {
		printf("%s:%d\n", __FILE__, __LINE__);
		exit(0);
	}
}

bool dos_t::set_in_env(uint16_t env_seg, const char *s) {
	mcb_t env_mcb(machine, env_seg - 1);
	assert(env_mcb.has_valid_signature());

	uint16_t env_capacity = 0x10 * env_mcb.size_in_paras();
	assert(env_capacity < 0x8000);

	dos_environment_t env = dos_environment_t::from_seg(machine, env_seg);
	bool r = env.set(s);

	return r;
}

void dos_t::remove_from_env(uint16_t env_seg, const char *s) {
	const char *p = strchr(s, '=');
	assert(p);

	size_t keylen = p - s;
	assert(keylen > 0);

	uint16_t env_i = 0;

	while (char c = machine->mem_read8(env_seg, env_i)) {
		uint16_t key_i = 0;
		while (c == s[key_i]) {
			key_i++;
			env_i++;
		}
	}
}

void dos_t::log_int(const char *func) {
	return;
	printf("%s\n", func);
	cpu->dump_state();
}

void dos_t::unimplemented_int(const char *func) {
	printf("unimplemented! %s\n", func);

	cpu->dump_state();

	exit(0);
}

void dos_t::save_user_state() {
	user_regs.ax    = cpu->ax;
	user_regs.bx    = cpu->bx;
	user_regs.cx    = cpu->cx;
	user_regs.dx    = cpu->dx;
	user_regs.si    = cpu->si;
	user_regs.di    = cpu->di;
	user_regs.bp    = cpu->bp;
	user_regs.ds    = cpu->ds;
	user_regs.es    = cpu->es;
	user_regs.flags = cpu->flags;
}

void dos_t::restore_user_state() {
	cpu->ax    = user_regs.ax;
	cpu->bx    = user_regs.bx;
	cpu->cx    = user_regs.cx;
	cpu->dx    = user_regs.dx;
	cpu->si    = user_regs.si;
	cpu->di    = user_regs.di;
	cpu->bp    = user_regs.bp;
	cpu->ds    = user_regs.ds;
	cpu->es    = user_regs.es;
}

void dos_t::clc() {
	user_regs.flags &= ~0x01;
}

void dos_t::stc() {
	user_regs.flags |= 0x01;
}

void dos_t::syscall_ok() {
	clc();
}

void dos_t::syscall_error(byte error_code) {
	printf("syscall error: %x\n", error_code);
	user_regs.ax = error_code;
	stc();
}

bool dos_t::validate_mcb_chain() {
	// printf("MCB Chain:\n");
	for (mcb_t mcb(machine, 0x0158);; mcb = mcb.next()) {
		// printf("MCB: %c seg:%04x sz:%04x owner:%04x\n", mcb.signature(), mcb.seg, mcb.size_in_paras(), mcb.get_owner_pid());

		if (!mcb.has_valid_signature()) {
			return false;
		}
		if (mcb.is_last()) {
			break;
		}
	}
	// printf("\n");
	return true;
}

uint16_t dos_t::allocate_memory(uint16_t requested_paras, uint16_t *out_max_paras) {
	uint16_t largest_available = 0;
	uint16_t first_fit_seg = 0x0000;

	for (mcb_t mcb(machine, 0x0158);; mcb = mcb.next()) {
		assert(mcb.has_valid_signature());
		// printf("MCB %04x: %d\n", mcb.seg, mcb.is_free());
		if (mcb.is_free()) {
			assert(mcb.coalesce_free_blocks());

			largest_available = std::max(largest_available, mcb.size_in_paras());
			if (mcb.size_in_paras() >= requested_paras) {
				if (!first_fit_seg) {
					first_fit_seg = mcb.seg;
				}
			}
		}
		if (mcb.is_last()) {
			break;
		}
	}

	if (largest_available < requested_paras) {
		if (out_max_paras) {
			*out_max_paras = largest_available;
		}
		return 0;
	}

	mcb_t mcb = mcb_t(machine, first_fit_seg);
	uint32_t mcb_size = mcb.size_in_paras();

	if (requested_paras < mcb_size) {
		mcb.split(requested_paras);
	}
	mcb.set_owner_pid(current_psp);

	return mcb.data_seg();
}
