#include "bios.h"

#include "emu/i8086.h"
#include "emu/ibm5160.h"

void bios_t::int1a() {
	switch (cpu->ax) {
		case 0x0000: int1a_00_read_system_clock_counter(); break;
		case 0x0001: int1a_01_set_system_clock_counter(); break;
		case 0x0002: int1a_02_read_real_time_clock_time(); break;
		case 0x0003: int1a_03_set_real_time_clock_time(); break;
		case 0x0004: int1a_04_read_real_time_clock_date(); break;
		case 0x0005: int1a_05_set_real_time_clock_date(); break;
		case 0x0006: int1a_06_set_real_time_clock_alarm(); break;
		case 0x0007: int1a_07_reset_real_time_clock_alarm(); break;
		case 0x0008: int1a_08_set_rtc_activated_power_on_mode(); break;
		case 0x0009: int1a_09_read_rtc_alarm_time_and_status(); break;
		case 0x000A: int1a_0a_read_system_day_counter(); break;
		case 0x000B: int1a_0b_set_system_day_counter(); break;
		case 0x0080: int1a_80_set_up_sound_multiplexer(); break;
	}

	cpu->op_iret();
}

void bios_t::int1a_00_read_system_clock_counter() {
	unimplemented_int(__FUNCTION__);
}

void bios_t::int1a_01_set_system_clock_counter() {
	unimplemented_int(__FUNCTION__);
}

void bios_t::int1a_02_read_real_time_clock_time() {
	unimplemented_int(__FUNCTION__);
}

void bios_t::int1a_03_set_real_time_clock_time() {
	unimplemented_int(__FUNCTION__);
}

void bios_t::int1a_04_read_real_time_clock_date() {
	unimplemented_int(__FUNCTION__);
}

void bios_t::int1a_05_set_real_time_clock_date() {
	unimplemented_int(__FUNCTION__);
}

void bios_t::int1a_06_set_real_time_clock_alarm() {
	unimplemented_int(__FUNCTION__);
}

void bios_t::int1a_07_reset_real_time_clock_alarm() {
	unimplemented_int(__FUNCTION__);
}

void bios_t::int1a_08_set_rtc_activated_power_on_mode() {
	unimplemented_int(__FUNCTION__);
}

void bios_t::int1a_09_read_rtc_alarm_time_and_status() {
	unimplemented_int(__FUNCTION__);
}

void bios_t::int1a_0a_read_system_day_counter() {
	unimplemented_int(__FUNCTION__);
}

void bios_t::int1a_0b_set_system_day_counter() {
	unimplemented_int(__FUNCTION__);
}

void bios_t::int1a_80_set_up_sound_multiplexer() {
	unimplemented_int(__FUNCTION__);
}
