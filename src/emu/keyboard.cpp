#include "keyboard.h"
#include "emu/ibm5160.h"
#include "emu/i8086.h"
#include <algorithm>
#include <queue>
#include <iostream>

keyboard_t::keyboard_t() {
}

// returns 0 if empty, or 1ms worth of cycles.
// something large enough for run_cycles to run and intr 9 to be raised.
uint64_t keyboard_t::next_cycles() {
	if (input_queue.empty()) {
		return 0;
	}
	return input_queue.size() * scan_code_set_1_length;
}

uint64_t keyboard_t::run_cycles(uint64_t cycles) {
	if (!input_queue.empty()) {
		glfw_input_key_t last_input_key = input_queue.front();
		input_queue.pop();
		for (int i = 0; i < scan_code_set_1_length; i++) {
			key_sequence_t element = scan_code_set_1[i];
			if (element.glfw_index == last_input_key.glfw_index) {
				if (!last_input_key.is_key_up) {
					for (byte value : element.make_sequence) {
						add_input_buffer(value);
						update_port_60_value(value);
					}
				}
				else {
					for (byte value : element.break_sequence) {
						add_input_buffer(value);
						update_port_60_value(value);
					}
				}
			}
		}
	}
	return input_queue.size();
}

void keyboard_t::update_port_60_value(byte value) {
	if (keyboard_memory.buffer_read_position <= 0) {
		return;
	}
	byte port_60_value = keyboard_memory.input_buffer[keyboard_memory.buffer_write_position];
	keyboard_memory.port_60_value = value;
	machine->cpu->raise_intr(9);
	keyboard_memory.buffer_write_position++;
	if (keyboard_memory.buffer_write_position >= INPUT_BUFFER_SIZE) {
		keyboard_memory.buffer_write_position -= INPUT_BUFFER_SIZE;
	}
	keyboard_memory.buffer_read_position--;
}

void keyboard_t::add_input_buffer(byte value) {
	if (keyboard_memory.buffer_read_position >= INPUT_BUFFER_SIZE) {
		return;
	}
	byte buffer_read_position = keyboard_memory.buffer_write_position + keyboard_memory.buffer_read_position;
	if (buffer_read_position >= INPUT_BUFFER_SIZE) {
		buffer_read_position -= INPUT_BUFFER_SIZE;
	}
	keyboard_memory.input_buffer[buffer_read_position] = value;
	keyboard_memory.buffer_read_position++;
}

uint8_t keyboard_t::read() {
	return keyboard_memory.port_60_value;
}

void keyboard_t::set_key_down(int down_key_id)
{
	input_queue.push(glfw_input_key_t(down_key_id, false));
}

void keyboard_t::set_key_up(int up_key_id)
{
	input_queue.push(glfw_input_key_t(up_key_id, true));
}
