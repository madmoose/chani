#include "dos_alloc.h"

#include "emu/ibm5160.h"

enum {
	M = 0x4d,
	Z = 0x5a,
};

mcb_t::mcb_t(ibm5160_t *machine, uint16_t seg):
	machine(machine), seg(seg)
{
	byte sig = signature();
	// assert(sig == M || sig == Z);
}

mcb_t mcb_t::build_mcb(ibm5160_t *machine, uint16_t seg) {
	machine->mem_write8(seg, 0, M);
	for (int i = 1; i != 16; ++i) {
		machine->mem_write8(seg, i, 0);
	}
	return mcb_t(machine, seg);
}

byte mcb_t::signature() {
	return machine->mem_read8(seg, 0);
}

bool mcb_t::has_valid_signature() {
	byte sig = signature();
	return sig == Z || sig == M;
}

bool mcb_t::is_last() {
	return signature() == Z;
}

void mcb_t::set_is_last(bool last) {
	return machine->mem_write8(seg, 0, !last ? M : Z);
}

uint16_t mcb_t::get_owner_pid() {
	return machine->mem_read16(seg, 1);
}

void mcb_t::set_owner_pid(uint16_t pid) {
	return machine->mem_write16(seg, 1, pid);
}

bool mcb_t::is_free() {
	return get_owner_pid() == 0;
}

void mcb_t::set_is_free() {
	set_owner_pid(0);
}

uint16_t mcb_t::size_in_paras() {
	return machine->mem_read16(seg, 3);
}

void mcb_t::set_size_in_paras(uint16_t paras) {
	return machine->mem_write16(seg, 3, paras);
}

uint16_t mcb_t::data_seg() {
	assert(seg < 0xffff);
	return seg + 1;
}

mcb_t mcb_t::next() {
	return mcb_t(machine, data_seg() + size_in_paras());
}

void mcb_t::split(uint16_t paras) {
	assert(paras < size_in_paras());

	uint16_t old_size_in_paras = size_in_paras();
	set_size_in_paras(paras);

	mcb_t new_mcb = build_mcb(machine, data_seg() + paras);
	new_mcb.set_size_in_paras(old_size_in_paras - paras - 1);

	if (is_last()) {
		set_is_last(false);
		new_mcb.set_is_last(true);
	}
}

void mcb_t::combine() {
	assert(!is_last());

	mcb_t next_mcb = next();
	set_size_in_paras(size_in_paras() + 1 + next_mcb.size_in_paras());
	set_is_last(next_mcb.is_last());
}

bool mcb_t::coalesce_free_blocks() {
	// Combine consecutive empty blocks
	while (!is_last()) {
		mcb_t next_mcb = next();
		if (!next_mcb.has_valid_signature()) {
			return false;
		}
		if (!next_mcb.is_free()) {
			return true;
		}
		combine();
	}
	return true;
}
