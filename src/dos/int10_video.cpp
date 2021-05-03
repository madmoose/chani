#include "dos.h"

#include "../emu/i8086.h"
#include "../emu/ibm5160.h"

void dos_t::int10() {
	byte ah = readhi(machine->cpu->ax);

	switch (ah) {
		case 0x00: int10_00_set_video_mode(); break;
		case 0x01: int10_01_set_text_mode_cursor_shape(); break;
		case 0x02: int10_02_set_cursor_position(); break;
		case 0x03: int10_03_get_cursor_position_and_shape(); break;
		case 0x04: int10_04_read_light_pen_position(); break;
		case 0x05: int10_05_select_active_display_page(); break;
		case 0x06: int10_06_scroll_up_window(); break;
		case 0x07: int10_07_scroll_down_window(); break;
		case 0x08: int10_08_read_character_and_attribute_at_cursor_position(); break;
		case 0x09: int10_09_write_character_and_attribute_at_cursor_position(); break;
		case 0x0A: int10_0a_write_character_only_at_cursor_position(); break;
		case 0x0B: int10_0b_set_color(); break;
		case 0x0C: int10_0c_write_graphics_pixel(); break;
		case 0x0D: int10_0d_read_graphics_pixel(); break;
		case 0x0E: int10_0e_teletype_output(); break;
		case 0x0F: int10_0f_get_current_video_mode(); break;
		case 0x11: int10_11_change_text_mode_character_set(); break;
		case 0x13: int10_13_write_string(); break;
	}
}

void dos_t::int10_00_set_video_mode() {
	// unimplemented_int(__FUNCTION__);
	printf("INT10: Set video mode %x\n", readlo(machine->cpu->ax));
}

void dos_t::int10_01_set_text_mode_cursor_shape() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int10_02_set_cursor_position() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int10_03_get_cursor_position_and_shape() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int10_04_read_light_pen_position() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int10_05_select_active_display_page() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int10_06_scroll_up_window() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int10_07_scroll_down_window() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int10_08_read_character_and_attribute_at_cursor_position() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int10_09_write_character_and_attribute_at_cursor_position() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int10_0a_write_character_only_at_cursor_position() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int10_0b_set_color() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int10_0c_write_graphics_pixel() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int10_0d_read_graphics_pixel() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int10_0e_teletype_output() {
	// unimplemented_int(__FUNCTION__);
	 byte c = readlo(machine->cpu->ax);
	 printf("%c", isprint(c) ? c : '.');
}

void dos_t::int10_0f_get_current_video_mode() {
	writehi(machine->cpu->ax, 80);
	writelo(machine->cpu->ax,  3);
	writehi(machine->cpu->bx,  0);
}

void dos_t::int10_11_change_text_mode_character_set() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int10_13_write_string() {
	unimplemented_int(__FUNCTION__);
}
