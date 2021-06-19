#include "keyboard.h"
#include <queue>

keyboard_t::keyboard_t() {
}

uint8_t keyboard_t::read() {
	if (input_queue.empty()) {
		return 0;
	}

	if (output_queue.empty()) {
		glfw_input_key_t last_input_key = input_queue.front();
		for (int i = 0; i < scan_code_set_1_length; i++) {
			key_sequence_t element = scan_code_set_1[i];
			if (element.glfw_index == last_input_key.glfw_index) {
				if (last_input_key.is_key_up) {
					output_queue = std::queue(element.break_sequence);
				}
				else {
					output_queue = std::queue(element.make_sequence);
				}
			}
		}
	}

	byte value = output_queue.front();
	if (!output_queue.empty()) {
		output_queue.pop();
	}
	if (!input_queue.empty()) {
		input_queue.pop();
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
