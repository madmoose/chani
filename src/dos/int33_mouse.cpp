#include "dos.h"

#include "emu/i8086.h"
#include "emu/ibm5160.h"

#include <algorithm>

void dos_t::int33() {
	switch (cpu->ax) {
		case 0x0000: int33_0000_reset_driver_and_read_status(); break;
		case 0x0001: int33_0001_show_mouse_cursor(); break;
		case 0x0002: int33_0002_hide_mouse_cursor(); break;
		case 0x0003: int33_0003_return_position_and_button_status(); break;
		case 0x0004: int33_0004_position_mouse_cursor(); break;
		case 0x0005: int33_0005_return_button_press_data(); break;
		case 0x0006: int33_0006_return_button_release_data(); break;
		case 0x0007: int33_0007_define_horizontal_cursor_range(); break;
		case 0x0008: int33_0008_define_vertical_cursor_range(); break;
		case 0x0009: int33_0009_define_graphics_cursor(); break;
		case 0x000a: int33_000a_define_text_cursor(); break;
		case 0x000b: int33_000b_read_motion_counters(); break;
		case 0x000c: int33_000c_define_interrupt_subroutine_parameters(); break;
		case 0x000d: int33_000d_light_pen_emulation_on(); break;
		case 0x000e: int33_000e_light_pen_emulation_off(); break;
		case 0x000f: int33_000f_define_mickey_pixel_ratio(); break;
		case 0x0010: int33_0010_define_screen_region_for_updating(); break;
	}

	cpu->op_iret();
}

void dos_t::set_mouse(uint16_t x, uint16_t y, uint16_t buttons) {
	mouse_x = std::clamp<uint16_t>(x, 0, 639);
	mouse_y = std::clamp<uint16_t>(y, 0, 199);
	mouse_buttons = buttons;
}

void dos_t::int33_0000_reset_driver_and_read_status() {
	// unimplemented_int(__FUNCTION__);
	cpu->ax = 0xffff;
	cpu->bx =      3;
}

void dos_t::int33_0001_show_mouse_cursor() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int33_0002_hide_mouse_cursor() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int33_0003_return_position_and_button_status() {
	cpu->cx = mouse_x & 0xfffe;
	cpu->dx = mouse_y;
	cpu->bx = mouse_buttons;
}

void dos_t::int33_0004_position_mouse_cursor() {
	mouse_x = cpu->cx;
	mouse_y = cpu->dx;

	// printf("mouse position: (%d, %d)\n", mouse_x, mouse_y);
}

void dos_t::int33_0005_return_button_press_data() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int33_0006_return_button_release_data() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int33_0007_define_horizontal_cursor_range() {
	printf("int33,7: horizontal mouse range %d-%d\n", cpu->cx, cpu->dx);
	// unimplemented_int(__FUNCTION__);
}

void dos_t::int33_0008_define_vertical_cursor_range() {
	printf("int33,8: vertical mouse range %d-%d\n", cpu->cx, cpu->dx);
	// unimplemented_int(__FUNCTION__);
}

void dos_t::int33_0009_define_graphics_cursor() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int33_000a_define_text_cursor() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int33_000b_read_motion_counters() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int33_000c_define_interrupt_subroutine_parameters() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int33_000d_light_pen_emulation_on() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int33_000e_light_pen_emulation_off() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int33_000f_define_mickey_pixel_ratio() {
	// unimplemented_int(__FUNCTION__);
}

void dos_t::int33_0010_define_screen_region_for_updating() {
	unimplemented_int(__FUNCTION__);
}
