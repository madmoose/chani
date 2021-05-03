#ifndef EMU_ISA_BUS
#define EMU_ISA_BUS

#include "support/types.h"

class isa_bus_t {
public:
	void write_byte(uint32_t address, byte value);
	byte read_byte(uint32_t address);
};

#endif
