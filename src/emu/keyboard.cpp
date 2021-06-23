#include "keyboard.h"
#include "emu/ibm5160.h"
#include "emu/i8086.h"
#include <algorithm>
#include <queue>
#include <iostream>
#include <string>
#include <vector>

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
					set_output_vector(element.break_sequence);
				}
				else {
					set_output_vector(element.make_sequence);
				}
			}
		}
		machine->cpu->raise_intr(9);
	}
	return output_vector.size() - output_index;
}

void keyboard_t::set_output_vector(std::list<byte> sequence) {
	output_vector = { std::begin(sequence), std::end(sequence) };
}

uint8_t keyboard_t::read() {

	byte value = 0;
	if (!output_vector.empty()) {
		if (output_index < output_vector.size() - 1) {
			output_index++;
		}
		value = output_vector.at(output_index);
	}

	return value;
}

void keyboard_t::set_key_down(int down_key_id)
{
	input_queue.push(glfw_input_key_t(down_key_id, false));
}

void keyboard_t::set_key_up(int up_key_id)
{
	input_queue.push(glfw_input_key_t(up_key_id, true));
}
