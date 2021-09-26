#ifndef EMU_IBM5160
#define EMU_IBM5160

#include "emu/emu.h"
#include "emu/machine.h"
#include "support/types.h"

#define MEMORY_SIZE     0x100000

class bios_t;
class dos_t;
class i8086_t;
class i8254_pit_t;
class vga_t;
class keyboard_t;

class ibm5160_t : public machine_t {
public:
	bios_t      *bios;
	dos_t       *dos;
	i8254_pit_t *pit;
	vga_t       *vga;
	keyboard_t  *keyboard;

	ibm5160_t();

	uint16_t read(address_space_t, uint32_t, width_t = W8);
	void     write(address_space_t, uint32_t, width_t, uint16_t);

	byte mem_read8(uint16_t seg, uint16_t ofs) {
		return read(MEM, 0x10 * seg + ofs, W8);
	}

	uint16_t mem_read16(uint16_t seg, uint16_t ofs) {
		return read(MEM, 0x10 * seg + ofs, W16);
	}

	void mem_write8(uint16_t seg, uint16_t ofs, byte v) {
		return write(MEM, 0x10 * seg + ofs, W8, v);
	}

	void mem_write16(uint16_t seg, uint16_t ofs, uint16_t v) {
		return write(MEM, 0x10 * seg + ofs, W16, v);
	}
};

#endif
