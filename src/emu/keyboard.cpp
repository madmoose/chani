#include "keyboard.h"
#include <algorithm>
#include <queue>
#include <iostream>
#include <string>

#ifndef DEBUG_KBD
#define DEBUG_KBD 0
#endif

keyboard_t::keyboard_t() {
}

void keyboard_t::push_input_sequence(std::list<byte> sequence)
{
#ifdef DEBUG_KBD
	for (byte value : sequence) {
		printf("%x", value);
	}
	printf("\n");
#endif
	for (byte value : sequence) {
		output_queue.push(value);
	}
}

uint8_t keyboard_t::read() {
	if (!input_queue.empty()) {
		glfw_input_key_t last_input_key = input_queue.front();
		input_queue.pop();
		for (int i = 0; i < scan_code_set_1_length; i++) {
			key_sequence_t element = scan_code_set_1[i];
			if (element.glfw_index == last_input_key.glfw_index) {
#ifdef DEBUG_KBD
				std::cout << element.key_name << ": ";
#endif
				if (!last_input_key.is_key_up) {
					push_input_sequence(element.break_sequence);
				}
				else {
					push_input_sequence(element.make_sequence);
				}
			}
		}
	}

	byte value = 0;
	if (!output_queue.empty()) {
		value = output_queue.front();
		output_queue.pop();
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
