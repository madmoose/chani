#include "bios.h"

#include "emu/i8086.h"
#include "emu/ibm5160.h"

void bios_t::int08() {
	uint32_t timer_lo, timer_hi;
	// byte timer_ofl;

	timer_lo  = machine->mem_read16(0x0040, 0x006c);
	timer_hi  = machine->mem_read16(0x0040, 0x006e);

	timer_lo += 1;
	if (timer_lo == 0) {
		timer_hi += 1;
	}

	machine->mem_write16(0x0040, 0x006c, timer_lo);
	machine->mem_write16(0x0040, 0x006e, timer_hi);

	// Test whether timer equals 24 hours
	if (timer_hi == 0x0018 && timer_lo == 0x00b0) {
		timer_lo = 0;
		timer_hi = 0;

		machine->mem_write16(0x0040, 0x006c, timer_lo);
		machine->mem_write16(0x0040, 0x006e, timer_hi);
		machine->mem_write8(0x0040, 0x0080, 1);
	}
	// TODO: Handle diskette motor countdown?

	// TODO: Tailcall int 1c, any way to handle this "properly"?
	cpu->ip = machine->mem_read16(0x0000, 4 * 0x1c);
	cpu->cs = machine->mem_read16(0x0000, 4 * 0x1c + 2);

	// TODO: EOI
}
