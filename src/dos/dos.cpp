#include "dos.h"

#include "emu/i8086.h"
#include "emu/ibm5160.h"
#include "support/file_reader.h"

void dos_t::install() {
	i8086_t *cpu = machine->cpu;

	cpu->install_callback(0x0000, 4 * 0x21, std::bind(&dos_t::int21, this));
	cpu->install_callback(0x0000, 4 * 0x33, std::bind(&dos_t::int33, this));
}

void dos_t::unimplemented_int(const char *func) {
	printf("unimplemented! %s\n", func);

	machine->cpu->dump_state();

	exit(0);
}
