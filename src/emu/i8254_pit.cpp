#include "emu/i8254_pit.h"

#include "emu/i8086.h"
#include "emu/ibm5160.h"

#include <cassert>
#include <algorithm>
#include <limits>

void i8254_counter_t::write(byte v) {
	switch (w) {
		case 0b00:
			return;
		case 0b01:
			writelo(count_register, v);
			w = 0b00;
			break;
		case 0b10:
			writehi(count_register, v);
			w = 0b00;
			break;
		case 0b11:
			writelo(count_register, v);
			w = 0b10;
			break;
		default:
			assert(0 && "unreachable");
	}

	if (w == 0b00) {
		activated = true;
		counting_element = count_register;
		printf("Count register set to 0x%04x\n", count_register);
		if (counting_element == 0) {
			counting_element = 0x10000;
		}
	}
}

byte i8254_counter_t::read() {
	switch (r) {
		case 0b00:
			return 0;
		case 0b01:
			r = 0b00;
			return readlo(output_latch);
		case 0b10:
			r = 0b00;
			return readhi(output_latch);
		case 0b11:
			r = 0b10;
			return readlo(output_latch);
		default:
			break;
	}
	assert(0 && "unreachable");
	return 0;
}

uint64_t i8254_counter_t::run_cycles(uint64_t cycles) {
	counting_element -= cycles;
	return cycles;
}

i8254_pit_t::i8254_pit_t() {
	counter[0].counting_element = 0x10000;
	counter[1].counting_element = 0x10000;
	counter[2].counting_element = 0x10000;

	counter[0].activated = true;
}

uint64_t i8254_pit_t::next_cycles() {
	uint32_t min_counter = 0xffffffff;
	for (int i = 0; i != 3; ++i) {
		if (counter[i].activated) {
			min_counter = std::min(min_counter, counter[0].counting_element);
		}
	}

	return min_counter;
}

uint64_t i8254_pit_t::run_cycles(uint64_t cycles) {
	cycles = std::min(cycles, next_cycles());
	for (int i = 0; i != 3; ++i) {
		counter[i].run_cycles(cycles);
	}
	if (counter[0].counting_element == 0) {
		machine->raise_intr(0x08);
		counter[0].counting_element = counter[0].count_register;
		if (counter[0].counting_element == 0) {
			counter[0].counting_element = 0x10000;
		}
	}
	return cycles;
}

byte i8254_pit_t::read(byte addr) {
	assert(addr <= 0b11);

	return counter[addr].output_latch;
}

void i8254_pit_t::write(byte addr, byte w) {
	assert(addr <= 0b11);

	printf("i8254_pit_t::write(%03x, %02x)\n", addr, w);

	if (addr < 0b11) {
		counter[addr].write(w);
	} else {
		// Address 0xbb: Control word
		byte sc = (w >> 6); // Select Counter
		if (sc < 3) {
			byte rw   = (w >> 4) & 0b11;
			byte mode = (w >> 1) & 0b111;
			bool bcd  = w & 1;

			if (rw == 0b00) { // Counter latch
				counter[sc].is_latched = true;
			} else {
				if (mode & 0b010) {
					mode |= 0b011;
				}

				counter[sc].r = rw;
				counter[sc].w = rw;
				counter[sc].mode = mode;
				counter[sc].bcd = bcd;

				counter[sc].out = (0b111110 >> mode);
			}
		} else {
			// Read-Back Command
		}
	}
}
