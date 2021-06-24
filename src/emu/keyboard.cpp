#include "keyboard.h"
#include "emu/ibm5160.h"
#include "emu/i8086.h"
#include <algorithm>
#include <queue>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#ifndef DEBUG_KBD
#define DEBUG_KBD 1
#endif

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
#ifdef DEBUG_KBD
				std::cout << element.key_name << "\n";
#endif
				if (!last_input_key.is_key_up) {
					for (byte data : element.make_sequence) {
						add_buffer(data);
						transfer_buffer(data);
					}
				}
				else {
					for (byte data : element.break_sequence) {
						add_buffer(data);
						transfer_buffer(data);
					}
				}
			}
		}
	}
	return input_queue.size();
}

void keyboard_t::transfer_buffer(byte val) {
	if (keyboard_memory.used_input_position <= 0) {
		return;
	}
	set_port_60(keyboard_memory.input_buffer[keyboard_memory.input_write_position]);
	keyboard_memory.input_write_position++;
	if (keyboard_memory.input_write_position >= INPUT_BUFFER_SIZE) {
		keyboard_memory.input_write_position -= INPUT_BUFFER_SIZE;
	}
	keyboard_memory.used_input_position--;
}

void keyboard_t::set_port_60(byte val) {
	keyboard_memory.p60data = val;
	machine->cpu->raise_intr(9);
}

void keyboard_t::add_buffer(byte data) {
	if (keyboard_memory.used_input_position >= INPUT_BUFFER_SIZE) {
		return;
	}
	byte start = keyboard_memory.input_write_position + keyboard_memory.used_input_position;
	if (start >= INPUT_BUFFER_SIZE) {
		start -= INPUT_BUFFER_SIZE;
	}
	keyboard_memory.input_buffer[start] = data;
	keyboard_memory.used_input_position++;
}

uint8_t keyboard_t::read() {
	return keyboard_memory.p60data;
}

void keyboard_t::set_key_down(int down_key_id)
{
	input_queue.push(glfw_input_key_t(down_key_id, false));
}

void keyboard_t::set_key_up(int up_key_id)
{
	input_queue.push(glfw_input_key_t(up_key_id, true));
}
