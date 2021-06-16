#ifndef EMU_KEYBOARD
#define EMU_KEYBOARD

#include "emu/device.h"
#include "support/types.h"

class keyboard_t {
public:
	keyboard_t();
	uint8_t read();
	void set_key_down(int key_id);
private:
	int key_id = 0;
};

#endif
