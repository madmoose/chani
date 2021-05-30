#include "bios.h"

#include "emu/ibm5160.h"
#include "emu/i8086.h"
#include "support/types.h"

#include <cstdio>
#include <cstdlib>

void bios_t::install() {
	/*
	 * Install an 'iret' command at 0xf0000 and use
	 * this address as the default interrupt handler.
	 */
	machine->cpu->mem_write8(0xf000, 0x0000, 0xcf); // iret
	for (int i = 0; i != 256; ++i) {
		machine->cpu->mem_write16(0x0000, 0x0000 + 4 * i + 0, 0x0000);
		machine->cpu->mem_write16(0x0000, 0x0000 + 4 * i + 2, 0xf000);
	}

	machine->cpu->set_callback_base(0xf001);
	machine->cpu->install_callback(0x0000, 4 * 0x08, std::bind(&bios_t::int08, this));
	machine->cpu->install_callback(0x0000, 4 * 0x10, std::bind(&bios_t::int10, this));

	writele16(&machine->memory[0x10 * 0x0040 + 0x0063], 0x03d4);
}

void bios_t::unimplemented_int(const char *func) {
	printf("unimplemented! %s\n", func);

	machine->cpu->dump_state();

	exit(0);
}
