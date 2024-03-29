#include <dos/dos.h>

#include "emu/i8086.h"
#include "emu/ibm5160.h"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <filesystem>

#define CHANIDEBUG 0

void dos_t::int21() {
	in_dos++;
	save_user_state();

	byte ah = readhi(cpu->ax);

	switch (ah) {
		case 0x00: int21_00_program_terminate(); break;
		case 0x01: int21_01_character_input(); break;
		case 0x02: int21_02_character_output(); break;
		case 0x03: int21_03_auxiliary_input(); break;
		case 0x04: int21_04_auxiliary_output(); break;
		case 0x05: int21_05_printer_output(); break;
		case 0x06: int21_06_direct_console_io(); break;
		case 0x07: int21_07_direct_console_input_without_echo(); break;
		case 0x08: int21_08_console_input_without_echo(); break;
		case 0x09: int21_09_display_string(); break;
		case 0x0a: int21_0a_buffered_keyboard_input(); break;
		case 0x0b: int21_0b_get_input_status(); break;
		case 0x0c: int21_0c_flush_input_buffer_and_input(); break;
		case 0x0d: int21_0d_disk_reset(); break;
		case 0x0e: int21_0e_set_default_drive(); break;
		case 0x0f: int21_0f_open_file(); break;
		case 0x10: int21_10_close_file(); break;
		case 0x11: int21_11_find_first_file(); break;
		case 0x12: int21_12_find_next_file(); break;
		case 0x13: int21_13_delete_file(); break;
		case 0x14: int21_14_sequential_read(); break;
		case 0x15: int21_15_sequential_write(); break;
		case 0x16: int21_16_create_or_truncate_file(); break;
		case 0x17: int21_17_rename_file(); break;
		case 0x18: int21_18_reserved(); break;
		case 0x19: int21_19_get_default_drive(); break;
		case 0x1a: int21_1a_set_disk_transfer_address(); break;
		case 0x1b: int21_1b_get_allocation_info_for_default_drive(); break;
		case 0x1c: int21_1c_get_allocation_info_for_specified_drive(); break;
		case 0x1d: int21_1d_reserved(); break;
		case 0x1e: int21_1e_reserved(); break;
		case 0x1f: int21_1f_get_disk_parameter_block_for_default_drive(); break;
		case 0x20: int21_20_reserved(); break;
		case 0x21: int21_21_random_read(); break;
		case 0x22: int21_22_random_write(); break;
		case 0x23: int21_23_get_file_size_in_records(); break;
		case 0x24: int21_24_set_random_record_number(); break;
		case 0x25: int21_25_set_interrupt_vector(); break;
		case 0x26: int21_26_create_psp(); break;
		case 0x27: int21_27_random_block_read(); break;
		case 0x28: int21_28_random_block_write(); break;
		case 0x29: int21_29_parse_filename(); break;
		case 0x2a: int21_2a_get_date(); break;
		case 0x2b: int21_2b_set_date(); break;
		case 0x2c: int21_2c_get_time(); break;
		case 0x2d: int21_2d_set_time(); break;
		case 0x2e: int21_2e_set_verify_flag(); break;
		case 0x2f: int21_2f_get_disk_transfer_address(); break;
		case 0x30: int21_30_get_dos_version(); break;
		case 0x31: int21_31_terminate_and_stay_resident(); break;
		case 0x32: int21_32_get_disk_parameter_block_for_specified_drive(); break;
		case 0x33: int21_33_get_or_set_ctrl_break(); break;
		case 0x34: int21_34_get_indos_flag_pointer(); break;
		case 0x35: int21_35_get_interrupt_vector(); break;
		case 0x36: int21_36_get_free_disk_space(); break;
		case 0x37: int21_37_get_or_set_switch_character(); break;
		case 0x38: int21_38_get_or_set_country_info(); break;
		case 0x39: int21_39_create_subdirectory(); break;
		case 0x3a: int21_3a_remove_subdirectory(); break;
		case 0x3b: int21_3b_change_current_directory(); break;
		case 0x3c: int21_3c_create_or_truncate_file(); break;
		case 0x3d: int21_3d_open_file(); break;
		case 0x3e: int21_3e_close_file(); break;
		case 0x3f: int21_3f_read_file_or_device(); break;
		case 0x40: int21_40_write_file_or_device(); break;
		case 0x41: int21_41_delete_file(); break;
		case 0x42: int21_42_move_file_pointer(); break;
		case 0x43: int21_43_get_or_set_file_attributes(); break;
		case 0x44: int21_44_io_control_for_devices(); break;
		case 0x45: int21_45_duplicate_handle(); break;
		case 0x46: int21_46_redirect_handle(); break;
		case 0x47: int21_47_get_current_directory(); break;
		case 0x48: int21_48_allocate_memory(); break;
		case 0x49: int21_49_release_memory(); break;
		case 0x4a: int21_4a_reallocate_memory(); break;
		case 0x4b: int21_4b_execute_program(); break;
		case 0x4c: int21_4c_terminate_with_return_code(); break;
		case 0x4d: int21_4d_get_program_return_code(); break;
		case 0x4e: int21_4e_find_first_file(); break;
		case 0x4f: int21_4f_find_next_file(); break;
		case 0x50: int21_50_set_current_psp(); break;
		case 0x51: int21_51_get_current_psp(); break;
		case 0x52: int21_52_get_dos_internal_pointers(); break;
		case 0x53: int21_53_create_disk_parameter_block(); break;
		case 0x54: int21_54_get_verify_flag(); break;
		case 0x55: int21_55_create_program_psp(); break;
		case 0x56: int21_56_rename_file(); break;
		case 0x57: int21_57_get_or_set_file_date_and_time(); break;
		case 0x58: int21_58_get_or_set_allocation_strategy(); break;
		case 0x59: int21_59_get_extended_error_info(); break;
		case 0x5a: int21_5a_create_unique_file(); break;
		case 0x5b: int21_5b_create_new_file(); break;
		case 0x5c: int21_5c_lock_or_unlock_file(); break;
		case 0x5d: int21_5d_file_sharing_functions(); break;
		case 0x5e: int21_5e_network_functions(); break;
		case 0x5f: int21_5f_network_redirection_functions(); break;
		case 0x60: int21_60_qualify_filename(); break;
		case 0x61: int21_61_reserved(); break;
		case 0x62: int21_62_get_current_psp(); break;
		case 0x63: int21_63_get_dbcs_lead_byte_table_pointer(); break;
		case 0x64: int21_64_set_wait_for_external_event_flag(); break;
		case 0x65: int21_65_get_extended_country_info(); break;
		case 0x66: int21_66_get_or_set_code_page(); break;
		case 0x67: int21_67_set_handle_count(); break;
		case 0x68: int21_68_commit_file(); break;
		case 0x69: int21_69_get_or_set_media_id(); break;
		case 0x6a: int21_6a_commit_file(); break;
		case 0x6b: int21_6b_reserved(); break;
		case 0x6c: int21_6c_extended_open_create_file(); break;
		default:
			unimplemented_int(__FUNCTION__);
	}

	in_dos--;
	restore_user_state();

	// Update user flags on stack, iret restores flags from stack
	cpu->mem_write16(cpu->ss, cpu->sp + 4, user_regs.flags);
	cpu->op_iret();
}

void dos_t::int21_00_program_terminate() {
	exit(0);
}

void dos_t::int21_01_character_input() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_02_character_output() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_03_auxiliary_input() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_04_auxiliary_output() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_05_printer_output() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_06_direct_console_io() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_07_direct_console_input_without_echo() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_08_console_input_without_echo() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_09_display_string() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_0a_buffered_keyboard_input() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_0b_get_input_status() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_0c_flush_input_buffer_and_input() {
	log_int(__FUNCTION__);
}

void dos_t::int21_0d_disk_reset() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_0e_set_default_drive() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_0f_open_file() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_10_close_file() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_11_find_first_file() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_12_find_next_file() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_13_delete_file() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_14_sequential_read() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_15_sequential_write() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_16_create_or_truncate_file() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_17_rename_file() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_18_reserved() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_19_get_default_drive() {
	writelo(user_regs.ax, 2);
}

void dos_t::int21_1a_set_disk_transfer_address() {
	log_int(__FUNCTION__);
	user_dta_ofs = cpu->dx;
	user_dta_seg = cpu->ds;
	return_syscall_ok();
}

void dos_t::int21_1b_get_allocation_info_for_default_drive() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_1c_get_allocation_info_for_specified_drive() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_1d_reserved() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_1e_reserved() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_1f_get_disk_parameter_block_for_default_drive() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_20_reserved() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_21_random_read() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_22_random_write() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_23_get_file_size_in_records() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_24_set_random_record_number() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_25_set_interrupt_vector() {
	log_int(__FUNCTION__);
	byte num = readlo(cpu->ax);
	machine->mem_write16(0, 4 * num + 0, cpu->dx);
	machine->mem_write16(0, 4 * num + 2, cpu->ds);
}

void dos_t::int21_26_create_psp() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_27_random_block_read() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_28_random_block_write() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_29_parse_filename() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_2a_get_date() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_2b_set_date() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_2c_get_time() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_2d_set_time() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_2e_set_verify_flag() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_2f_get_disk_transfer_address() {
	log_int(__FUNCTION__);

	cpu->bx = user_dta_ofs;
	cpu->es = user_dta_seg;

	user_regs.bx = cpu->bx;
	user_regs.es = cpu->es;
	return_syscall_ok();
}

void dos_t::int21_30_get_dos_version() {
	log_int(__FUNCTION__);
	user_regs.bx = 0xff00; // bh    = oem number (ff = MS-DOS)
	user_regs.cx = 0;    // bl:cx = user number

	// DOS version 2.11
	writelo(user_regs.ax,  5);
	writehi(user_regs.ax,  0);
}

void dos_t::int21_31_terminate_and_stay_resident() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_32_get_disk_parameter_block_for_specified_drive() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_33_get_or_set_ctrl_break() {
	log_int(__FUNCTION__);
	switch (readlo(cpu->ax)) {
		case 0x00: // get ctrl-break checking flag
			writelo(user_regs.dx, ctrl_break ? 1 : 0);
			break;
		case 0x01: // set ctrl-break checking flag
			ctrl_break = !!readlo(cpu->dx);
			break;
		default:
			unimplemented_int(__FUNCTION__);
	}
}

void dos_t::int21_34_get_indos_flag_pointer() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_35_get_interrupt_vector() {
	log_int(__FUNCTION__);
	// unimplemented_int(__FUNCTION__);
	// cpu->es = 0xc7ff;
	// cpu->bx = 0x0010;
	// printf("[%04x:%04x] Getting interrupt vector %02xh\n", cpu->cs, cpu->ip, readlo(cpu->ax));

	byte num = readlo(cpu->ax);

	if (num == 0) {
		user_regs.bx = 0xe4cc;
		user_regs.es = 0xf000;
		return;
	}

	user_regs.bx = machine->mem_read16(0, 4 * num + 0);
	user_regs.es = machine->mem_read16(0, 4 * num + 2);
}

void dos_t::int21_36_get_free_disk_space() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_37_get_or_set_switch_character() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_38_get_or_set_country_info() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_39_create_subdirectory() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_3a_remove_subdirectory() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_3b_change_current_directory() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_3c_create_or_truncate_file() {
	log_int(__FUNCTION__);
	char filepath[257];

	int len = 0;
	uint16_t ds = cpu->ds;
	uint16_t dx = cpu->dx;
	byte c;

	while (len < 256 && (c = machine->read(MEM, 0x10*ds + dx++))) {
		filepath[len++] = c;
	}
	filepath[len] = '\0';

	printf("Creating file '%s'\n", filepath);

	FILE *f = fopen(filepath, "w+b");
	assert(f);

	int fd = open_files.size() + first_fd;
	open_files.push_back(f);

	user_regs.ax = fd;
	return_syscall_ok();
}

void dos_t::int21_3d_open_file() {
	log_int(__FUNCTION__);
	char filepath[257];

	int len = 0;
	uint16_t ds = cpu->ds;
	uint16_t dx = cpu->dx;
	byte c;

	int start_line = std::max(0, (dx - 32) / 16);
	for (int i = start_line; i != start_line + 5; ++i) {
		printf("%04x:%04x ", ds, 0x10 * i);
		for (int j = 0; j != 16; ++j) {
			printf("%02x ", machine->mem_read8(ds, 0x10 * i + j));
		}
		for (int j = 0; j != 16; ++j) {
			byte c = machine->mem_read8(ds, 0x10 * i + j);
			printf("%c", isprint(c) ? c : '.');
		}
		printf("\n");
	}

	printf("====== %c\n", machine->mem_read8(ds, dx-1));

	while (len < 256 && (c = machine->read(MEM, 0x10*ds + dx++))) {
		filepath[len++] = c;
	}
	filepath[len] = '\0';

	printf("Opening file '%s'\n", filepath);

	// Compare the requested filepath with all the files in the current
	// directory, case-insensitively.
	// TODO: Support and search folders
	bool found_file = false;
	std::filesystem::path path;
	for (auto &p: std::filesystem::directory_iterator(".")) {
		path = p.path();
		std::string path_str = path.filename().string();

		bool equal = std::equal(
			path_str.begin(), path_str.end(),
			filepath, filepath+len,
			[](char a, char b) {
				return tolower(a) == tolower(b);
			}
		);
		if (equal) {
			found_file = true;
			break;
		}
	}

	if (!found_file) {
		printf("File '%s' not found.\n", filepath);
		return_syscall_error(error_file_not_found);
		return;
	}

#ifdef _MSC_VER
	FILE *f = _wfopen(path.c_str(), L"rb");
#else
	FILE *f = fopen(path.c_str(), "rb");
#endif
	assert(f);

	printf("Found file '%s'.\n", filepath);

	int fd = open_files.size() + first_fd;
	open_files.push_back(f);

	user_regs.ax = fd;
	return_syscall_ok();
}

void dos_t::int21_3e_close_file() {
	log_int(__FUNCTION__);
	FILE *f = open_files.at(cpu->bx - first_fd);

	assert(f);

	int r = fclose(f);
	if (r != 0) {
		return_syscall_error(error_invalid_handle);
	}

	return_syscall_ok();
}

void dos_t::int21_3f_read_file_or_device() {
	// log_int(__FUNCTION__);
	FILE     *f     = open_files.at(cpu->bx - first_fd);
	uint16_t  count = cpu->cx;
	byte     *buf   = machine->memory + (0x10 * cpu->ds + cpu->dx);

	assert(f);
	size_t r = fread(buf, 1, count, f);

	user_regs.ax = r;
	return_syscall_ok();
}

void dos_t::int21_40_write_file_or_device() {
	log_int(__FUNCTION__);
	FILE     *f = open_files.at(cpu->bx - first_fd);
	uint16_t  count = cpu->cx;
	byte     *buf   = machine->memory + (0x10 * cpu->ds + cpu->dx);

	assert(f);
	uint16_t bytes_written = fwrite(buf, 1, count, f);

	user_regs.ax = bytes_written;
	return_syscall_ok();
}

void dos_t::int21_41_delete_file() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_42_move_file_pointer() {
	// log_int(__FUNCTION__);
	FILE *f = open_files.at(cpu->bx - first_fd);
	size_t offset = (((uint32_t)cpu->cx) << 16) + cpu->dx;

	assert(f);

	int whence;
	switch (readlo(cpu->ax)) {
		case 0: whence = SEEK_SET; break;
		case 1: whence = SEEK_CUR; break;
		case 2: whence = SEEK_END; break;
		default:
			return_syscall_error(error_invalid_function);
	}

	size_t result = fseek(f, offset, whence);
	assert(result == 0);

	size_t position = ftell(f);

	user_regs.dx = (position >> 16);
	user_regs.ax = (position >>  0) & 0xffff;
	return_syscall_ok();
}

void dos_t::int21_43_get_or_set_file_attributes() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_44_io_control_for_devices() {
	log_int(__FUNCTION__);
	// printf("IOCTL al:%d, bx=%d\n", readlo(cpu->ax), cpu->bx);
	// unimplemented_int(__FUNCTION__);
	user_regs.dx = 0x0080;
	return_syscall_ok();
}

void dos_t::int21_45_duplicate_handle() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_46_redirect_handle() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_47_get_current_directory() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_48_allocate_memory() {
	log_int(__FUNCTION__);
	uint16_t requested_paras = cpu->bx;
	uint16_t largest_available = 0;
	uint16_t first_fit_seg = 0x0000;
	uint16_t best_fit_seg  = 0x0000;
	uint16_t best_fit_size = 0x0000;
	uint16_t last_fit_seg  = 0x0000;

	if (!validate_mcb_chain()) {
		return_syscall_error(error_arena_trashed);
	}

	for (mcb_t mcb(machine, 0x0158);; mcb = mcb.next()) {
		if (!mcb.has_valid_signature()) {
			break;
		}
		if (mcb.is_free()) {
			/* If there are multiple consecutive free blocks,
			   combine them into one free block before proceeding. */
			if (!mcb.coalesce_free_blocks()) {
				return_syscall_error(error_arena_trashed);
			}

			uint16_t free_size = mcb.size_in_paras();

			largest_available = std::max(largest_available, free_size);

			if (free_size >= requested_paras) {
				if (first_fit_seg == 0) {
					first_fit_seg = mcb.seg;
				}

				if (!best_fit_seg || free_size < best_fit_size) {
					best_fit_size = free_size;
					best_fit_seg = mcb.seg;
				}

				last_fit_seg = mcb.seg;
			}
		}

		if (mcb.is_last()) {
			break;
		}
	}

	if (largest_available < requested_paras) {
		user_regs.bx = largest_available;
		return_syscall_error(error_not_enough_memory);
	}

	uint32_t block_seg;
	switch (allocation_strategy) {
		case 0:
			block_seg = first_fit_seg;
			break;
		case 1:
			block_seg = best_fit_seg;
			break;
		default:
			block_seg = last_fit_seg;
			break;
	}

	mcb_t mcb = mcb_t(machine, block_seg);
	uint32_t mcb_size = mcb.size_in_paras();

	if (requested_paras < mcb_size) {
		if (allocation_strategy < 2) {
			mcb.split(requested_paras);
		} else {
			/* If strategy is Last Fit then split such
			 * that the new allocation fits at the end. */
			mcb.split(mcb_size - requested_paras - 1);
			mcb = mcb.next();
		}
	}
	mcb.set_owner_pid(1);
	user_regs.ax = mcb.data_seg();

	if (!validate_mcb_chain()) {
		return_syscall_error(error_arena_trashed);
	}

	return_syscall_ok();
}

void dos_t::int21_49_release_memory() {
	if (!validate_mcb_chain()) {
		return_syscall_error(error_arena_trashed);
	}

	uint16_t seg = cpu->bx;
	assert(seg > 0);

	mcb_t mcb(machine, seg - 1);
	if (!mcb.has_valid_signature()) {
		return_syscall_error(error_invalid_block);
	}

	mcb.set_is_free();
}

void dos_t::int21_4a_reallocate_memory() {
	log_int(__FUNCTION__);

	uint16_t segment         = cpu->es;
	uint16_t requested_paras = cpu->bx;

	if (!validate_mcb_chain()) {
		printf("%s:%d\n", __FILE__, __LINE__);
		exit(0);
		return_syscall_error(error_arena_trashed);
	}

	mcb_t mcb(machine, segment - 1);

	if (!mcb.coalesce_free_blocks()) {
		printf("%s:%d\n", __FILE__, __LINE__);
		exit(0);
		return_syscall_error(error_arena_trashed);
	}

	uint16_t mcb_size = mcb.size_in_paras();

	if (requested_paras > mcb_size) {
		user_regs.bx = mcb_size;
		return_syscall_error(error_not_enough_memory);
	}

	if (requested_paras < mcb_size) {
		mcb.split(requested_paras);
	}

	validate_mcb_chain();

	user_regs.ax = segment;
	return_syscall_ok();
}

void dos_t::int21_4b_execute_program() {
	unimplemented_int(__FUNCTION__);

	uint8_t method = readlo(cpu->ax);
	if (method != 0 && method != 1 && method != 3) {
		return_syscall_error(error_invalid_function);
	}

	int21_3d_open_file();
	exit(0);
}

void dos_t::int21_4c_terminate_with_return_code() {
	log_int(__FUNCTION__);
	exit(0);
}

void dos_t::int21_4d_get_program_return_code() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_4e_find_first_file() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_4f_find_next_file() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_50_set_current_psp() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_51_get_current_psp() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_52_get_dos_internal_pointers() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_53_create_disk_parameter_block() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_54_get_verify_flag() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_55_create_program_psp() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_56_rename_file() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_57_get_or_set_file_date_and_time() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_58_get_or_set_allocation_strategy() {
	log_int(__FUNCTION__);

	switch (readlo(cpu->ax)) {
		// Get allocation strategy
		case 0:
			user_regs.ax = allocation_strategy;
			return_syscall_ok();

		// Set allocation strategy
		case 1:
			allocation_strategy = readlo(cpu->bx);
			return_syscall_ok();
	}
	return_syscall_error(error_invalid_function);
}

void dos_t::int21_59_get_extended_error_info() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_5a_create_unique_file() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_5b_create_new_file() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_5c_lock_or_unlock_file() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_5d_file_sharing_functions() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_5e_network_functions() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_5f_network_redirection_functions() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_60_qualify_filename() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_61_reserved() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_62_get_current_psp() {
	log_int(__FUNCTION__);
	user_regs.dx = current_psp;
	return_syscall_ok();
}

void dos_t::int21_63_get_dbcs_lead_byte_table_pointer() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_64_set_wait_for_external_event_flag() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_65_get_extended_country_info() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_66_get_or_set_code_page() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_67_set_handle_count() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_68_commit_file() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_69_get_or_set_media_id() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_6a_commit_file() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_6b_reserved() {
	unimplemented_int(__FUNCTION__);
}

void dos_t::int21_6c_extended_open_create_file() {
	unimplemented_int(__FUNCTION__);
}
