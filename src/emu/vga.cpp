#include "emu/vga.h"

#include "emu/ibm5160.h"

#include <cstdio>

vga_t::vga_t() {
	h_total = h_visible_area + h_front_porch + h_sync_pulse + h_back_porch;
	v_total = v_visible_area + v_front_porch + v_sync_pulse + v_back_porch;

	total_pels  = h_total * v_total;
	v_sync_pels = h_total * v_sync_pulse;

	memset(dac_ram, 0, sizeof(dac_ram));
}

uint64_t vga_t::next_cycles() {
	int total_pels  = h_total * v_total;
	int v_sync_pels = h_total * v_sync_pulse;

	while (current_pel >= total_pels) {
		current_pel -= total_pels;
	}

	if (current_pel < v_sync_pels) {
		return v_sync_pels - current_pel;
	}

	return total_pels - current_pel;
}

uint64_t vga_t::run_cycles(uint64_t cycles) {
	current_pel += cycles;
	while (current_pel >= total_pels) {
		current_pel -= total_pels;
	}

	if (current_pel < v_sync_pels) {
		return v_sync_pels - current_pel;
	}

	if (current_pel == v_sync_pels) {
		// write_ppm(0xa0000, 320, 200);
	}

	return total_pels - current_pel;
}

bool vga_t::frame_ready() {
	return current_pel == v_sync_pels;
}

void vga_t::read_rgba(byte *p, uint32_t addr, int w, int h) {
	static int frame_number;
	static int next_frame_number = 0;
	char filename[32];

	frame_number = next_frame_number++;

	for (int y = 0; y != h; ++y) {
		for (int x = 0; x != w; ++x) {
			int offset = w * y + x;
			byte c = machine->memory[addr + offset];
			byte r = dac_ram[3*c+0];
			byte g = dac_ram[3*c+1];
			byte b = dac_ram[3*c+2];
			p[4 * offset + 0] = (r << 2) | (r >> 4);
			p[4 * offset + 1] = (g << 2) | (g >> 4);
			p[4 * offset + 2] = (b << 2) | (b >> 4);
			p[4 * offset + 3] = 255;
		}
	}
}

void vga_t::read_dac_ram(byte *p) {
	memcpy(p, dac_ram, 0x300);
}

byte frame[320*200*3];

void vga_t::write_ppm(uint32_t addr, int w, int h) {
	static int frame_number;
	static int next_frame_number = 0;
	char filename[32];

	frame_number = next_frame_number++;

	for (int y = 0; y != h; ++y) {
		for (int x = 0; x != w; ++x) {
			int offset = w * y + x;
			byte c = machine->memory[addr + offset];
			byte r = dac_ram[3*c+0];
			byte g = dac_ram[3*c+1];
			byte b = dac_ram[3*c+2];
			frame[3 * offset + 0] = (r << 2) | (r >> 4);
			frame[3 * offset + 1] = (g << 2) | (g >> 4);
			frame[3 * offset + 2] = (b << 2) | (b >> 4);
		}
	}

	sprintf(filename, "ppm/frame-%05d.ppm", frame_number);
	FILE *f = fopen(filename, "w");
	if (!f) {
		static bool did_error = false;
		if (!did_error) {
			printf("\n\nCreate directory ppm/ to dump frames in.\n\n\n");
			did_error = true;
		}
		return;
	}
	if (frame_number % 70 == 0) {
		printf("Dumping framebuffer to %s\n", filename);
	}
	fprintf(f, "P6\n");
	fprintf(f, "%d %d\n%d\n", w, h, 255);
	fwrite(frame, 320*200*3, 1, f);
	fclose(f);
}

uint8_t vga_t::read(address_space_t address_space, uint32_t addr) {
	uint8_t v = 0;

	if (address_space == IO) {
		switch (addr) {
			case 0x3c7: // DAC State Register
				v = dac_state;
				break;
			case 0x3c8: // DAC Address Write Mode Register
				v = dac_address;
				break;
			case 0x3c9: // DAC Data Register
				v = dac_ram[dac_address++];
				dac_address %= 0x300;
				break;
			case 0x3da: // Input Status #1 Register
				v = 0;
				if (current_pel < v_sync_pels) {
					v |= 0b00001000;
				}
				break;
		}
		// printf("VGA: read  0x%3x -> %02x\n", addr, v);
	}

	return v;
}

void vga_t::write(address_space_t address_space, uint32_t addr, uint8_t v) {
	if (address_space == IO) {
		// printf("VGA: write %02x -> 0x%3x\n", v, addr);
		switch (addr) {
			case 0x3c7: // DAC Address Read Mode Register
				dac_address = 3 * v;
				dac_state = 0b00;
				break;
			case 0x3c8: // DAC Address Write Mode Register
				dac_address = 3 * v;
				dac_state = 0b11;
				break;
			case 0x3c9: // DAC Data Register
				dac_ram[dac_address++] = v & 0b111111;
				dac_address %= 0x300;
				break;
		}
	}
}
