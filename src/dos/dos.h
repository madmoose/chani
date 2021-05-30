#ifndef DOS_DOS_H
#define DOS_DOS_H

#include "support/types.h"

class file_reader_t;
class ibm5160_t;

class dos_t {
public:
	ibm5160_t *machine = nullptr;

	bool ctrl_break = true;

public:
	void unimplemented_int(const char *op_name);

	void install();

	void build_psp(uint16_t psp_segment);
	bool exec(file_reader_t &rd);

	void int21();
	void int33();

	void stc();
	void clc();

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
