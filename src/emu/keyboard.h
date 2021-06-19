#ifndef EMU_KEYBOARD
#define EMU_KEYBOARD

#include "emu/device.h"
#include "support/types.h"
#include <string>
#include <queue>
#include <list>

struct glfw_input_key_t {
	int  glfw_index; //From glfw3.h. Key to match input with scan code
	bool is_key_up; //to know if we return the make or the break sequence after finding a match
};

struct key_sequence_t {
	const std::string key_name; //for debug puposes
	int               glfw_index; //From glfw3.h. Key to match input with scan code
	std::list<byte>   make_sequence; //for key_down. Up to 6 bytes for PAUSE with scan code set 1.
	std::list<byte>   break_sequence; //for key_up. Up to 2 bytes with scan code set 1.
};

class keyboard_t {
public:
	keyboard_t();
	uint8_t read();
	void set_key_down(int input_key_id);
	void set_key_up(int input_key_id);
private:
	//a make or break sequence can be several bytes, but the CPU reads one byte at a time.
	std::queue<byte, std::list<byte>> output_queue = std::queue<byte, std::list<byte>>();

	const static int scan_code_set_1_length = 120;

	//taken from http://users.utcluj.ro/~baruch/sie/labor/PS2/Scan_Codes_Set_1.htm
	//Scan Code Set 1, sorted by glfw key values.
	key_sequence_t scan_code_set_1[scan_code_set_1_length] = {
		{"GLFW_KEY_SPACE", 32, std::list<byte>(1, 0x39), std::list<byte>(1, 0xB9)},
		{"GLFW_KEY_APOSTROPHE", 39, std::list<byte>(1, 0x28), std::list<byte>(1, 0xA8)},
		{"GLFW_KEY_COMMA", 44, std::list<byte>(1, 0x33), std::list<byte>(1, 0xB3)},
		{"GLFW_KEY_MINUS", 45, std::list<byte>(1, 0x0C), std::list<byte>(1, 0x8C)},
		{"GLFW_KEY_PERIOD", 45, std::list<byte>(1, 0x34), std::list<byte>(1, 0xB4)},
		{"GLFW_KEY_SLASH", 47, std::list<byte>(1, 0x35), std::list<byte>(1, 0xB5)},
		{"GLFW_KEY_0", 48, std::list<byte>(1, 0x52), std::list<byte>(1, 0xD2)},
		{"GLFW_KEY_1", 49, std::list<byte>(1, 0x4F), std::list<byte>(1, 0xCF)},
		{"GLFW_KEY_2", 50, std::list<byte>(1, 0x50), std::list<byte>(1, 0xD0)},
		{"GLFW_KEY_3", 51, std::list<byte>(1, 0x51), std::list<byte>(1, 0xD1)},
		{"GLFW_KEY_4", 52, std::list<byte>(1, 0x4B), std::list<byte>(1, 0xCB)},
		{"GLFW_KEY_5", 53, std::list<byte>(1, 0x4C), std::list<byte>(1, 0xCC)},
		{"GLFW_KEY_6", 54, std::list<byte>(1, 0x4D), std::list<byte>(1, 0xCD)},
		{"GLFW_KEY_7", 55, std::list<byte>(1, 0x47), std::list<byte>(1, 0xC7)},
		{"GLFW_KEY_8", 56, std::list<byte>(1, 0x48), std::list<byte>(1, 0xC8)},
		{"GLFW_KEY_9", 57, std::list<byte>(1, 0x49), std::list<byte>(1, 0xC9)},
		{"GLFW_KEY_SEMICOLON", 59, std::list<byte>(1, 0x27), std::list<byte>(1, 0xA7)},
		{"GLFW_KEY_EQUAL", 61, std::list<byte>(1, 0x0D), std::list<byte>(1, 0x8D)},
		{"GLFW_KEY_A", 65, std::list<byte>(1, 0x1E), std::list<byte>(1, 0x9E)},
		{"GLFW_KEY_B", 66, std::list<byte>(1, 0x30), std::list<byte>(1, 0xB0)},
		{"GLFW_KEY_C", 67, std::list<byte>(1, 0x2E), std::list<byte>(1, 0xAE)},
		{"GLFW_KEY_D", 68, std::list<byte>(1, 0x20), std::list<byte>(1, 0xA0)},
		{"GLFW_KEY_E", 69, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F", 70, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_G", 71, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_H", 72, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_I", 73, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_J", 74, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_K", 75, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_L", 76, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_M", 77, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_N", 78, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_O", 79, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_P", 80, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_Q", 81, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_R", 82, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_S", 83, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_T", 84, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_U", 85, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_V", 86, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_W", 87, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_X", 88, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_Y", 89, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_Z", 90, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_LEFT_BRACKET", 91, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_BACKSLASH", 92, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_RIGHT_BRACKET", 93, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_GRAVE_ACCENT", 96, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_WORLD_1", 161, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_WORLD_2", 162, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_ESCAPE", 256, std::list<byte>(1, 0x01), std::list<byte>(1, 0x81)},
		{"GLFW_KEY_ENTER", 257, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_TAB", 258, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_BACKSPACE", 259, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_INSERT", 260, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_DELETE", 261, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_RIGHT", 262, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_LEFT", 263, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_DOWN", 264, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_UP", 265, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_PAGE_UP", 266, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_PAGE_DOWN", 267, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_HOME", 268, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_END", 269, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_CAPS_LOCK", 280, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_SCROLL_LOCK", 281, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_NUM_LOCK", 282, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_PRINT_SCREEN", 283, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_PAUSE", 284, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F1", 290, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F2", 291, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F3", 292, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F4", 293, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F5", 294, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F6", 295, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F7", 296, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F8", 297, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F9", 298, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F10", 299, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F11", 300, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F12", 301, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F13", 302, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F14", 303, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F15", 304, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F16", 305, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F17", 306, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F18", 307, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F19", 308, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F20", 309, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F21", 310, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F22", 311, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F23", 312, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F24", 313, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_F25", 314, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_KP_0", 320, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_KP_1", 321, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_KP_2", 322, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_KP_3", 323, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_KP_4", 324, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_KP_5", 325, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_KP_6", 326, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_KP_7", 327, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_KP_8", 328, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_KP_9", 329, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_KP_DECIMAL", 330, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_KP_DIVIDE", 331, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_KP_MULTIPLY", 332, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_KP_SUBTRACT", 333, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_KP_ADD", 334, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_KP_ENTER", 335, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_KP_EQUAL", 336, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_LEFT_SHIFT", 340, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_LEFT_CONTROL", 341, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_LEFT_ALT", 342, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_LEFT_SUPER", 343, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_RIGHT_SHIFT", 344, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_RIGHT_CONTROL", 345, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_RIGHT_ALT", 346, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_RIGHT_SUPER", 347, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)},
		{"GLFW_KEY_MENU", 348, std::list<byte>(1, 0x00), std::list<byte>(1, 0x00)}
	};
	std::queue<glfw_input_key_t> input_queue = std::queue<glfw_input_key_t>();
};

#endif
