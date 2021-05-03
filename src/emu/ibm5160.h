#ifndef EMU_IBM5160
#define EMU_IBM5160

#include "support/types.h"

class dos_t;
class isa_bus_t;
class i8086_t;

class ibm5160_t {
public:
	isa_bus_t *bus;
	i8086_t   *cpu;
	dos_t     *dos;
	byte      *memory;

	ibm5160_t();

	byte     read_mem8(uint16_t seg, uint16_t ofs);
	uint16_t read_mem16(uint16_t seg, uint16_t ofs);

	void handle_software_interrupt(byte interrupt);
};

#endif
