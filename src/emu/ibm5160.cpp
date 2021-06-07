#include "ibm5160.h"

#include <cstdio>
#include <cstring>
#include <cassert>

#include "support/types.h"

#include "bios/bios.h"
#include "dos/dos.h"
#include "emu/i8086.h"
#include "emu/i8254_pit.h"
#include "emu/vga.h"

#define MEMORY_SIZE     0x100000
#define DIRTY_PAGE_SIZE     4096

ibm5160_t::ibm5160_t() {
	memory = (byte *)std::aligned_alloc(0x1000, MEMORY_SIZE);
	memset(memory, 0, MEMORY_SIZE);

	cpu = new i8086_t;
	cpu->set_machine(this);
	cpu->read  = THIS_READ_CB(read);
	cpu->write = THIS_WRITE_CB(write);

	pit = new i8254_pit_t;
	pit->set_machine(this);

	vga = new vga_t;
	vga->set_machine(this);

	bios = new bios_t;
	bios->machine = this;
	bios->install();

	dos  = new dos_t;
	dos->machine = this;
	dos->install();
}

extern void dump_call_stack();

uint16_t ibm5160_t::read(address_space_t address_space, uint32_t addr, width_t w) {
	if (address_space == MEM) {
		assert(addr < MEMORY_SIZE);
		if (w == W8) {
			return memory[addr];
		}
		return readle16(&memory[addr]);
	}

	if (address_space == IO) {
		assert(w == W8);
		byte v = 0;
		if (addr >= 0x3c0 && addr < 0x3e0) {
			v = vga->read(address_space, addr);
		}
		// printf("[%04x:%04x] IO %x -> %02x\n", cpu->cs, cpu->ip, addr, v);
		return v;
	}

	return 0;
}

void ibm5160_t::write(address_space_t address_space, uint32_t addr, width_t w, uint16_t v) {
	if (address_space == MEM) {
		assert(addr < MEMORY_SIZE);
		if (w == W8) {
			memory[addr] = v;
		} else {
			writele16(&memory[addr], v);
		}
		return;
	}

	if (address_space == IO) {
		// printf("[%04x:%04x] IO %x <- %02x\n", cpu->cs, cpu->ip, addr, v);
		assert(w == W8);

		if (addr >= 0x040 && addr < 0x060) {
			pit->write(addr - 0x040, v);
		} else if (addr >= 0x3c0 && addr < 0x3e0) {
			vga->write(address_space, addr, v);
		}
	}
}
