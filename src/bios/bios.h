#ifndef BIOS_BIOS_H

class ibm5160_t;

class bios_t {
public:
	ibm5160_t *machine = nullptr;

	void install();
	void unimplemented_int(const char *func);

	void int08();
	void int10();
	void int1a();

	void int10_00_set_video_mode();
	void int10_01_set_text_mode_cursor_shape();
	void int10_02_set_cursor_position();
	void int10_03_get_cursor_position_and_shape();
	void int10_04_read_light_pen_position();
	void int10_05_select_active_display_page();
	void int10_06_scroll_up_window();
	void int10_07_scroll_down_window();
	void int10_08_read_character_and_attribute_at_cursor_position();
	void int10_09_write_character_and_attribute_at_cursor_position();
	void int10_0a_write_character_only_at_cursor_position();
	void int10_0b_set_color();
	void int10_0c_write_graphics_pixel();
	void int10_0d_read_graphics_pixel();
	void int10_0e_teletype_output();
	void int10_0f_get_current_video_mode();
	void int10_11_change_text_mode_character_set();
	void int10_13_write_string();

	void int1a_00_read_system_clock_counter();
	void int1a_01_set_system_clock_counter();
	void int1a_02_read_real_time_clock_time();
	void int1a_03_set_real_time_clock_time();
	void int1a_04_read_real_time_clock_date();
	void int1a_05_set_real_time_clock_date();
	void int1a_06_set_real_time_clock_alarm();
	void int1a_07_reset_real_time_clock_alarm();
	void int1a_08_set_rtc_activated_power_on_mode();
	void int1a_09_read_rtc_alarm_time_and_status();
	void int1a_0a_read_system_day_counter();
	void int1a_0b_set_system_day_counter();
	void int1a_80_set_up_sound_multiplexer();
};

#endif
