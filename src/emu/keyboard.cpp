#include "keyboard.h"

keyboard_t::keyboard_t() {
}

uint8_t keyboard_t::read() {
	// From Scan Code Set 1
	// http://users.utcluj.ro/~baruch/sie/labor/PS2/Scan_Codes_Set_1.htm
	switch (key_id)
	{
	case 256: //Escape
		return 1;
		break;
	default:
		return key_id;
		break;
	}
	return 1;
}

void keyboard_t::set_key_down(int input_key_id)
{
	key_id = input_key_id;
}
