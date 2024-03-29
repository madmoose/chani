#ifndef DOS_DOS_H
#define DOS_DOS_H

#include <cstdio>
#include <vector>

#include "dos/dos_alloc.h"

#include "support/types.h"

class file_reader_t;
class ibm5160_t;
class i8086_t;

#define return_syscall_ok()       do { syscall_ok();       return; } while(0)
#define return_syscall_error(err) do { syscall_error(err); return; } while(0)

class dos_t {
public:
	ibm5160_t *machine = nullptr;
	i8086_t   *cpu;

	int in_dos = 0;
	bool ctrl_break = true;

	std::vector<FILE *> open_files;
	const int first_fd = 3;

	uint16_t initial_mcb_seg     = 0x0158;
	uint8_t  allocation_strategy = 0;

	uint16_t mouse_x = 0;
	uint16_t mouse_y = 0;
	uint16_t mouse_buttons = 0;

	uint16_t current_psp;

	uint16_t user_dta_ofs;
	uint16_t user_dta_seg;

	struct {
		uint16_t ax;
		uint16_t bx;
		uint16_t cx;
		uint16_t dx;
		uint16_t si;
		uint16_t di;
		uint16_t bp;
		uint16_t ds;
		uint16_t es;
		uint16_t flags;
	} user_regs;

	enum {
		error_invalid_function    =  1,
		error_file_not_found      =  2,
		error_path_not_found      =  3,
		error_too_many_open_files =  4,
		error_access_denied       =  5,
		error_invalid_handle      =  6,
		error_arena_trashed       =  7,
		error_not_enough_memory   =  8,
		error_invalid_block       =  9,
		error_bad_environment     = 10,
		error_bad_format          = 11,
		error_invalid_access      = 12,
		error_invalid_data        = 13,
		error_invalid_drive       = 15,
		error_current_directory   = 16,
		error_not_same_device     = 17,
		error_no_more_files       = 18,
	};

public:
	void log_int(const char *op_name);
	void unimplemented_int(const char *op_name);

	void install();

	void build_psp(uint16_t psp_segment, uint16_t psp_size_paras);
	bool exec(file_reader_t &rd);

	void set_mouse(uint16_t x, uint16_t y, uint16_t buttons);

	bool set_in_env(uint16_t env_seg, const char *s);
	void remove_from_env(uint16_t env_seg, const char *s);

	void int21();
	void int33();

	void save_user_state();
	void restore_user_state();

	void clc();
	void stc();

	void syscall_ok();
	void syscall_error(byte error_code);

	bool     validate_mcb_chain();
	uint16_t allocate_memory(uint16_t requested_paras, uint16_t *max_paras = nullptr);

	void int21_00_program_terminate();                            // 1.0+
	void int21_01_character_input();                              // 1.0+
	void int21_02_character_output();                             // 1.0+
	void int21_03_auxiliary_input();                              // 1.0+
	void int21_04_auxiliary_output();                             // 1.0+
	void int21_05_printer_output();                               // 1.0+
	void int21_06_direct_console_io();                            // 1.0+
	void int21_07_direct_console_input_without_echo();            // 1.0+
	void int21_08_console_input_without_echo();                   // 1.0+
	void int21_09_display_string();                               // 1.0+
	void int21_0a_buffered_keyboard_input();                      // 1.0+
	void int21_0b_get_input_status();                             // 1.0+
	void int21_0c_flush_input_buffer_and_input();                 // 1.0+
	void int21_0d_disk_reset();                                   // 1.0+
	void int21_0e_set_default_drive();                            // 1.0+
	void int21_0f_open_file();                                    // 1.0+
	void int21_10_close_file();                                   // 1.0+
	void int21_11_find_first_file();                              // 1.0+
	void int21_12_find_next_file();                               // 1.0+
	void int21_13_delete_file();                                  // 1.0+
	void int21_14_sequential_read();                              // 1.0+
	void int21_15_sequential_write();                             // 1.0+
	void int21_16_create_or_truncate_file();                      // 1.0+
	void int21_17_rename_file();                                  // 1.0+
	void int21_18_reserved();                                     // 1.0+
	void int21_19_get_default_drive();                            // 1.0+
	void int21_1a_set_disk_transfer_address();                    // 1.0+
	void int21_1b_get_allocation_info_for_default_drive();        // 1.0+
	void int21_1c_get_allocation_info_for_specified_drive();      // 1.0+
	void int21_1d_reserved();                                     // 1.0+
	void int21_1e_reserved();                                     // 1.0+
	void int21_1f_get_disk_parameter_block_for_default_drive();   // 1.0+
	void int21_20_reserved();                                     // 1.0+
	void int21_21_random_read();                                  // 1.0+
	void int21_22_random_write();                                 // 1.0+
	void int21_23_get_file_size_in_records();                     // 1.0+
	void int21_24_set_random_record_number();                     // 1.0+
	void int21_25_set_interrupt_vector();                         // 1.0+
	void int21_26_create_psp();                                   // 1.0+
	void int21_27_random_block_read();                            // 1.0+
	void int21_28_random_block_write();                           // 1.0+
	void int21_29_parse_filename();                               // 1.0+
	void int21_2a_get_date();                                     // 1.0+
	void int21_2b_set_date();                                     // 1.0+
	void int21_2c_get_time();                                     // 1.0+
	void int21_2d_set_time();                                     // 1.0+
	void int21_2e_set_verify_flag();                              // 1.0+
	void int21_2f_get_disk_transfer_address();                    // 2.0+
	void int21_30_get_dos_version();                              // 2.0+
	void int21_31_terminate_and_stay_resident();                  // 2.0+
	void int21_32_get_disk_parameter_block_for_specified_drive(); // 2.0+
	void int21_33_get_or_set_ctrl_break();                        // 2.0+
	void int21_34_get_indos_flag_pointer();                       // 2.0+
	void int21_35_get_interrupt_vector();                         // 2.0+
	void int21_36_get_free_disk_space();                          // 2.0+
	void int21_37_get_or_set_switch_character();                  // 2.0+
	void int21_38_get_or_set_country_info();                      // 2.0+
	void int21_39_create_subdirectory();                          // 2.0+
	void int21_3a_remove_subdirectory();                          // 2.0+
	void int21_3b_change_current_directory();                     // 2.0+
	void int21_3c_create_or_truncate_file();                      // 2.0+
	void int21_3d_open_file();                                    // 2.0+
	void int21_3e_close_file();                                   // 2.0+
	void int21_3f_read_file_or_device();                          // 2.0+
	void int21_40_write_file_or_device();                         // 2.0+
	void int21_41_delete_file();                                  // 2.0+
	void int21_42_move_file_pointer();                            // 2.0+
	void int21_43_get_or_set_file_attributes();                   // 2.0+
	void int21_44_io_control_for_devices();                       // 2.0+
	void int21_45_duplicate_handle();                             // 2.0+
	void int21_46_redirect_handle();                              // 2.0+
	void int21_47_get_current_directory();                        // 2.0+
	void int21_48_allocate_memory();                              // 2.0+
	void int21_49_release_memory();                               // 2.0+
	void int21_4a_reallocate_memory();                            // 2.0+
	void int21_4b_execute_program();                              // 2.0+
	void int21_4c_terminate_with_return_code();                   // 2.0+
	void int21_4d_get_program_return_code();                      // 2.0+
	void int21_4e_find_first_file();                              // 2.0+
	void int21_4f_find_next_file();                               // 2.0+
	void int21_50_set_current_psp();                              // 2.0+
	void int21_51_get_current_psp();                              // 2.0+
	void int21_52_get_dos_internal_pointers();                    // 2.0+
	void int21_53_create_disk_parameter_block();                  // 2.0+
	void int21_54_get_verify_flag();                              // 2.0+
	void int21_55_create_program_psp();                           // 2.0+
	void int21_56_rename_file();                                  // 2.0+
	void int21_57_get_or_set_file_date_and_time();                // 2.0+
	void int21_58_get_or_set_allocation_strategy();               // 2.11+
	void int21_59_get_extended_error_info();                      // 3.0+
	void int21_5a_create_unique_file();                           // 3.0+
	void int21_5b_create_new_file();                              // 3.0+
	void int21_5c_lock_or_unlock_file();                          // 3.0+
	void int21_5d_file_sharing_functions();                       // 3.0+
	void int21_5e_network_functions();                            // 3.0+
	void int21_5f_network_redirection_functions();                // 3.0+
	void int21_60_qualify_filename();                             // 3.0+
	void int21_61_reserved();                                     // 3.0+
	void int21_62_get_current_psp();                              // 3.0+
	void int21_63_get_dbcs_lead_byte_table_pointer();             // 3.0+
	void int21_64_set_wait_for_external_event_flag();             // 3.2+
	void int21_65_get_extended_country_info();                    // 3.3+
	void int21_66_get_or_set_code_page();                         // 3.3+
	void int21_67_set_handle_count();                             // 3.3+
	void int21_68_commit_file();                                  // 3.3+
	void int21_69_get_or_set_media_id();                          // 4.0+
	void int21_6a_commit_file();                                  // 4.0+
	void int21_6b_reserved();                                     // 4.0+
	void int21_6c_extended_open_create_file();                    // 4.0+

	void int33_0000_reset_driver_and_read_status();
	void int33_0001_show_mouse_cursor();
	void int33_0002_hide_mouse_cursor();
	void int33_0003_return_position_and_button_status();
	void int33_0004_position_mouse_cursor();
	void int33_0005_return_button_press_data();
	void int33_0006_return_button_release_data();
	void int33_0007_define_horizontal_cursor_range();
	void int33_0008_define_vertical_cursor_range();
	void int33_0009_define_graphics_cursor();
	void int33_000a_define_text_cursor();
	void int33_000b_read_motion_counters();
	void int33_000c_define_interrupt_subroutine_parameters();
	void int33_000d_light_pen_emulation_on();
	void int33_000e_light_pen_emulation_off();
	void int33_000f_define_mickey_pixel_ratio();
	void int33_0010_define_screen_region_for_updating();
};

#endif
