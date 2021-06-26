#include "keyboard.h"

#include "emu/i8086.h"
#include "emu/ibm5160.h"

#include <algorithm>
#include <iostream>
#include <queue>

keyboard_t::keyboard_t() :
	data_output_buffer(0),
	status(0),
	next_event(0)
{
	glfw_key_state.reset();
}

// Returns 0 if empty, or 1ms worth of cycles.
// Something large enough for run_cycles to run and intr 9 to be raised.
uint64_t keyboard_t::next_cycles() {
	if (next_event > 0) {
		return next_event;
	}

	if (buffer.empty()) {
		next_event = 0;
	} else {
		next_event = 1000 * frequency_in_mhz();
	}
	return next_event;
}

uint64_t keyboard_t::run_cycles(uint64_t cycles) {
	next_event -= cycles;

	if (next_event == 0) {
		if (!buffer.empty()) {
			data_output_buffer = buffer.front();
			buffer.pop_front();
			status |= I8042_STATUS_OUTPUT_BUFFER_FULL;
			machine->cpu->raise_intr(9);
		}
	}

	return cycles;
}

uint8_t keyboard_t::read(uint8_t addr) {
	switch (addr) {
		case 0x00:
			if (status | I8042_STATUS_OUTPUT_BUFFER_FULL) {
				status &= ~I8042_STATUS_OUTPUT_BUFFER_FULL;
				next_event = 1000 * frequency_in_mhz();
			}
			return data_output_buffer;
		case 0x04:
			return status;
		default:
			printf("keyboard: unhandled io read @ %02x -> %02x\n", addr, 0);
	}
	return 0;
}

void keyboard_t::write(uint8_t addr, uint8_t v) {
	printf("keyboard: unhandled io write @ %02x <- %02x\n", addr, v);
}

void keyboard_t::set_key_down(int key_id) {
	if (glfw_key_state.test(key_id)) {
		return;
	}

	glfw_key_state.set(key_id);

	for (int i = 0; i < scan_code_set_1_length; i++) {
		key_sequence_t element = scan_code_set_1[i];
		if (element.glfw_index == key_id) {
			for (byte value : element.make_sequence) {
				buffer.push_back(value);
			}
			break;
		}
	}
}

void keyboard_t::set_key_up(int key_id) {
	if (!glfw_key_state.test(key_id)) {
		return;
	}

	glfw_key_state.reset(key_id);

	for (int i = 0; i < scan_code_set_1_length; i++) {
		key_sequence_t element = scan_code_set_1[i];
		if (element.glfw_index == key_id) {
			for (byte value : element.break_sequence) {
				buffer.push_back(value);
			}
			break;
		}
	}
}
