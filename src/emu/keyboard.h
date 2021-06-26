#ifndef EMU_KEYBOARD
#define EMU_KEYBOARD

#include "emu/device.h"
#include "support/types.h"

#include <GLFW/glfw3.h>

#include <bitset>
#include <list>
#include <queue>
#include <string>

struct glfw_input_key_t {
	int  glfw_index; // From glfw3.h. Key to match input with scan code
	bool is_key_up;  // to know if we return the make or the break sequence after finding a match
};

struct key_sequence_t {
	const std::string key_name;       // For debug purposes
	int               glfw_index;     // From glfw3.h. Key to match input with scan code
	std::list<byte>   make_sequence;  // For key_down. Up to 6 bytes for PAUSE with scan code set 1.
	std::list<byte>   break_sequence; // For key_up. Up to 2 bytes with scan code set 1.
};

#define I8042_STATUS_OUTPUT_BUFFER_FULL 0x01

class keyboard_t : public device_t {
public:
	keyboard_t();

	double   frequency_in_mhz() { return 20.0; };
	uint64_t next_cycles();
	uint64_t run_cycles(uint64_t cycles);

	uint8_t read(uint8_t addr);
	void    write(uint8_t addr, uint8_t v);

	void set_key_down(int input_key_id);
	void set_key_up(int input_key_id);

private:
	uint64_t next_event;

	std::bitset<GLFW_KEY_LAST+1> glfw_key_state;

	// Translated keys in scan code format
	std::deque<uint8_t> buffer;

	byte data_output_buffer;
	byte status;

	const static int scan_code_set_1_length = 104;

	// Taken from http://users.utcluj.ro/~baruch/sie/labor/PS2/Scan_Codes_Set_1.htm
	// Scan Code Set 1, sorted by glfw key values.
	key_sequence_t scan_code_set_1[scan_code_set_1_length] = {
		{"GLFW_KEY_SPACE", 32, {0x39}, {0xB9}},
		{"GLFW_KEY_APOSTROPHE", 39, {0x28}, {0xA8}},
		{"GLFW_KEY_COMMA", 44, {0x33}, {0xB3}},
		{"GLFW_KEY_MINUS", 45, {0x0C}, {0x8C}},
		{"GLFW_KEY_PERIOD", 45, {0x34}, {0xB4}},
		{"GLFW_KEY_SLASH", 47, {0x35}, {0xB5}},
		{"GLFW_KEY_0", 48, {0x0B}, {0x8B}},
		{"GLFW_KEY_1", 49, {0x02}, {0x82}},
		{"GLFW_KEY_2", 50, {0x03}, {0x83}},
		{"GLFW_KEY_3", 51, {0x04}, {0x84}},
		{"GLFW_KEY_4", 52, {0x05}, {0x85}},
		{"GLFW_KEY_5", 53, {0x06}, {0x86}},
		{"GLFW_KEY_6", 54, {0x07}, {0x87}},
		{"GLFW_KEY_7", 55, {0x08}, {0x88}},
		{"GLFW_KEY_8", 56, {0x09}, {0x89}},
		{"GLFW_KEY_9", 57, {0x0A}, {0x8A}},
		{"GLFW_KEY_SEMICOLON", 59, {0x27}, {0xA7}},
		{"GLFW_KEY_EQUAL", 61, {0x0D}, {0x8D}},
		{"GLFW_KEY_A", 65, {0x1E}, {0x9E}},
		{"GLFW_KEY_B", 66, {0x30}, {0xB0}},
		{"GLFW_KEY_C", 67, {0x2E}, {0xAE}},
		{"GLFW_KEY_D", 68, {0x20}, {0xA0}},
		{"GLFW_KEY_E", 69, {0x12}, {0x92}},
		{"GLFW_KEY_F", 70, {0x21}, {0xA1}},
		{"GLFW_KEY_G", 71, {0x22}, {0xA2}},
		{"GLFW_KEY_H", 72, {0x23}, {0xA3}},
		{"GLFW_KEY_I", 73, {0x17}, {0x92}},
		{"GLFW_KEY_J", 74, {0x24}, {0xA4}},
		{"GLFW_KEY_K", 75, {0x25}, {0xA5}},
		{"GLFW_KEY_L", 76, {0x26}, {0xA6}},
		{"GLFW_KEY_M", 77, {0x32}, {0xB2}},
		{"GLFW_KEY_N", 78, {0x31}, {0xB1}},
		{"GLFW_KEY_O", 79, {0x18}, {0x98}},
		{"GLFW_KEY_P", 80, {0x19}, {0x99}},
		{"GLFW_KEY_Q", 81, {0x10}, {0x90}},
		{"GLFW_KEY_R", 82, {0x13}, {0x93}},
		{"GLFW_KEY_S", 83, {0x1F}, {0x9F}},
		{"GLFW_KEY_T", 84, {0x14}, {0x94}},
		{"GLFW_KEY_U", 85, {0x16}, {0x96}},
		{"GLFW_KEY_V", 86, {0x2F}, {0xAF}},
		{"GLFW_KEY_W", 87, {0x11}, {0x91}},
		{"GLFW_KEY_X", 88, {0x2D}, {0xAD}},
		{"GLFW_KEY_Y", 89, {0x15}, {0x95}},
		{"GLFW_KEY_Z", 90, {0x2C}, {0xAC}},
		{"GLFW_KEY_LEFT_BRACKET", 91, {0x1A}, {0x9A}},
		{"GLFW_KEY_BACKSLASH", 92, {0x2B}, {0xAB}},
		{"GLFW_KEY_RIGHT_BRACKET", 93, {0x1B}, {0x9B}},
		{"GLFW_KEY_GRAVE_ACCENT", 96, {0x29}, {0x89}},
		{"GLFW_KEY_ESCAPE", 256, {0x01}, {0x81}},
		{"GLFW_KEY_ENTER", 257, {0x1C}, {0x9C}},
		{"GLFW_KEY_TAB", 258, {0x0F}, {0x8F}},
		{"GLFW_KEY_BACKSPACE", 259, {0x0E}, {0x8E}},
		{"GLFW_KEY_INSERT", 260, {0xE0, 0x52}, {0xE0, 0xD2}},
		{"GLFW_KEY_DELETE", 261, {0xE0, 0x53}, {0xE0, 0xD3}},
		{"GLFW_KEY_RIGHT", 262, {0xE0, 0x4D}, {0xE0, 0xCD}},
		{"GLFW_KEY_LEFT", 263, {0xE0, 0x4B}, {0xE0, 0xCB}},
		{"GLFW_KEY_DOWN", 264, {0xE0, 0x50}, {0xE0, 0xD0}},
		{"GLFW_KEY_UP", 265, {0xE0, 0x48}, {0xE0, 0xC8}},
		{"GLFW_KEY_PAGE_UP", 266, {0xE0, 0x49}, {0xE0, 0xC9}},
		{"GLFW_KEY_PAGE_DOWN", 267, {0xE0, 0x51}, {0xE0, 0xD1}},
		{"GLFW_KEY_HOME", 268, {0x0E, 0x47}, {0xE0, 0x97}},
		{"GLFW_KEY_END", 269, {0xE0, 0x4F}, {0xE0, 0xCF}},
		{"GLFW_KEY_CAPS_LOCK", 280, {0x3A}, {0xBA}},
		{"GLFW_KEY_SCROLL_LOCK", 281, {0x46}, {0xC6}},
		{"GLFW_KEY_NUM_LOCK", 282, {0x45}, {0xC5}},
		{"GLFW_KEY_PRINT_SCREEN", 283, {0xE0, 0x2A, 0xE0, 0x37}, {0xE0, 0xB7, 0xE0, 0xAA}},
		{"GLFW_KEY_PAUSE", 284, {0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5}, {}},
		{"GLFW_KEY_F1", 290, {0x3B}, {0xBB}},
		{"GLFW_KEY_F2", 291, {0x3C}, {0xBC}},
		{"GLFW_KEY_F3", 292, {0x3D}, {0xBD}},
		{"GLFW_KEY_F4", 293, {0x3E}, {0xBE}},
		{"GLFW_KEY_F5", 294, {0x3F}, {0xBF}},
		{"GLFW_KEY_F6", 295, {0x40}, {0xC0}},
		{"GLFW_KEY_F7", 296, {0x41}, {0xC1}},
		{"GLFW_KEY_F8", 297, {0x42}, {0xC2}},
		{"GLFW_KEY_F9", 298, {0x43}, {0xC3}},
		{"GLFW_KEY_F10", 299, {0x44}, {0xC4}},
		{"GLFW_KEY_F11", 300, {0x57}, {0xD7}},
		{"GLFW_KEY_F12", 301, {0x58}, {0xD8}},
		{"GLFW_KEY_KP_0", 320, {0x52}, {0xD2}},
		{"GLFW_KEY_KP_1", 321, {0x4F}, {0xCF}},
		{"GLFW_KEY_KP_2", 322, {0x50}, {0xD0}},
		{"GLFW_KEY_KP_3", 323, {0x51}, {0xD1}},
		{"GLFW_KEY_KP_4", 324, {0x4B}, {0xCB}},
		{"GLFW_KEY_KP_5", 325, {0x4C}, {0xCC}},
		{"GLFW_KEY_KP_6", 326, {0x4D}, {0xCD}},
		{"GLFW_KEY_KP_7", 327, {0x47}, {0xC7}},
		{"GLFW_KEY_KP_8", 328, {0x48}, {0xC8}},
		{"GLFW_KEY_KP_9", 329, {0x49}, {0xC9}},
		{"GLFW_KEY_KP_DECIMAL", 330, {0x53}, {0xD3}},
		{"GLFW_KEY_KP_DIVIDE", 331, {0xE0, 0x35}, {0xE0, 0xB5}},
		{"GLFW_KEY_KP_MULTIPLY", 332, {0x37}, {0xB7}},
		{"GLFW_KEY_KP_SUBTRACT", 333, {0x4A}, {0xCA}},
		{"GLFW_KEY_KP_ADD", 334, {0x4E}, {0xCE}},
		{"GLFW_KEY_KP_ENTER", 335, {0xE0, 0x1C}, {0xE0, 0x9C}},
		{"GLFW_KEY_LEFT_SHIFT", 340, {0x2A}, {0xAA}},
		{"GLFW_KEY_LEFT_CONTROL", 341, {0x1D}, {0x9D}},
		{"GLFW_KEY_LEFT_ALT", 342, {0x38}, {0xB8}},
		{"GLFW_KEY_LEFT_SUPER", 343, {0xE0, 0x5B}, {0xE0, 0xDB}},
		{"GLFW_KEY_RIGHT_SHIFT", 344, {0x36}, {0xB6}},
		{"GLFW_KEY_RIGHT_CONTROL", 345, {0xE0, 0x1D}, {0x00}},
		{"GLFW_KEY_RIGHT_ALT", 346, {0xE0, 0x38}, {0xE0, 0xB8}},
		{"GLFW_KEY_RIGHT_SUPER", 347, {0xE0, 0x5C}, {0xE0, 0xDC}},
		{"GLFW_KEY_MENU", 348, {0xE0, 0x5D}, {0xE0, 0xDD}}
	};
};

#endif
