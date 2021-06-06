#ifndef EMU_VGA_H
#define EMU_VGA_H

#include "emu/emu.h"
#include "emu/device.h"

class vga_t : public device_t {
	int h_visible_area = 640;
	int h_front_porch  =  16;
	int h_sync_pulse   =  96;
	int h_back_porch   =  48;

	int v_visible_area = 400;
	int v_front_porch  =  12;
	int v_sync_pulse   =   2;
	int v_back_porch   =  35;

	int h_total;
	int v_total;

	int total_pels;
	int v_sync_pels;

	int current_pel = 0;

	uint8_t  dac_state = 0;
	uint16_t dac_address = 0;
	uint8_t  dac_ram[0x300];

public:
	vga_t();

	double frequency_in_mhz() {
		return 25.175;
	}
	uint64_t next_cycles();
	uint64_t run_cycles(uint64_t cycles);

	bool frame_ready();

	void read_rgba(byte *p, uint32_t addr, int w, int h);
	void read_dac_ram(byte *p);
	void write_ppm(uint32_t addr, int w, int h);

	uint8_t read(address_space_t, uint32_t);
	void    write(address_space_t, uint32_t, uint8_t);
};

#endif
