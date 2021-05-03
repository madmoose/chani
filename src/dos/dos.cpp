#include "dos.h"
#include "../emu/i8086.h"
#include "../emu/ibm5160.h"

void dos_t::unimplemented_int(const char *func) {
	printf("unimplemented! %s\n", func);

	machine->cpu->dump_state();

	exit(0);
}
