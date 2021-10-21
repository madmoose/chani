#include "bios.h"

#include "emu/ibm5160.h"
#include "emu/i8086.h"
#include "support/types.h"

#include <cstdio>
#include <cstdlib>

void bios_t::install() {
	cpu = (i8086_t*)machine->cpu;
	/*
	 * Install an 'iret' command at 0xf0000 and use
	 * this address as the default interrupt handler.
	 */
	cpu->mem_write8(0xf000, 0x0000, 0xcf); // iret
	for (int i = 0; i != 256; ++i) {
		cpu->mem_write16(0x0000, 0x0000 + 4 * i + 0, 0x0000);
		cpu->mem_write16(0x0000, 0x0000 + 4 * i + 2, 0xf000);
	}

	cpu->set_callback_base(0xf001);
	cpu->install_callback(0x0000, 4 * 0x08, std::bind(&bios_t::int08, this));
	cpu->install_callback(0x0000, 4 * 0x10, std::bind(&bios_t::int10, this));

	writele16(&machine->memory[0x10 * 0x0040 + 0x0063], 0x03d4);
}

void bios_t::unimplemented_int(const char *func) {
	printf("unimplemented! %s\n", func);

	cpu->dump_state();

	exit(0);
}
