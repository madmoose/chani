#include "ibm5160.h"

#include <cstdio>

#include "support/types.h"

#include "dos/dos.h"
#include "emu/isa_bus.h"
#include "emu/i8086.h"

ibm5160_t::ibm5160_t()
	: memory(new byte[640*1024])
{
	for (int i = 0; i != 640*1024; ++i) {
		memory[i] = 0;
	}

	cpu = new i8086_t();
	cpu->set_machine(this);
	cpu->set_memory(memory);
}

byte ibm5160_t::read_mem8(uint16_t seg, uint16_t ofs) {
	return memory[0x10 * seg + ofs];
}

uint16_t ibm5160_t::read_mem16(uint16_t seg, uint16_t ofs) {
	byte b0 = read_mem8(seg, ofs);
	byte b1 = read_mem8(seg, ofs+1);

	uint16_t w = (uint16_t(b0) << 0u)
	           + (uint16_t(b1) << 8u);

	return w;
}

void ibm5160_t::handle_software_interrupt(byte interrupt) {
	switch (interrupt) {
		case 0x10: dos->int10(); break;
		case 0x21: dos->int21(); break;
		case 0x33: dos->int33(); break;
		default:
			printf("%s:%d Unhandled interrupt %x\n", __FUNCTION__, __LINE__, interrupt);
			// exit(0);
	}
}
