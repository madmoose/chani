#ifndef EMU_KEYBOARD
#define EMU_KEYBOARD

#include "emu/device.h"
#include "support/types.h"
#include <string>
#include <queue>
#include <list>

struct glfw_input_key_t {
	int  glfw_index; //key to match input with scan code
	bool is_key_up; //to know if we return the make or the break sequence after finding a match
};

struct key_sequence_t {
	int               glfw_index; //key to match input with scan code
	const std::string key_name; //for debug puposes
	std::list<byte>  make_sequence; //for key_down. Up to 6 bytes for PAUSE with scan code set 1.
	std::list<byte>  break_sequence; //for key_up. Up to 2 bytes with scan code set 1.
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

	//taken from http://users.utcluj.ro/~baruch/sie/labor/PS2/Scan_Codes_Set_1.htm
	const static int scan_code_set_1_length = 104;
	key_sequence_t scan_code_set_1[scan_code_set_1_length] = {
		{256, "Esc", std::list<byte>(1, 0x1), std::list<byte>(1, 0x81)}
	};
	std::queue<glfw_input_key_t> input_queue = std::queue<glfw_input_key_t>();
};

#endif
