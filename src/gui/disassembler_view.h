#ifndef GUI_DISASSEMBLER_VIEW_H
#define GUI_DISASSEMBLER_VIEW_H

#include <optional>
#include <string>
#include <deque>

#include "emu/emu.h"
#include "support/types.h"

class disasm_i8086_t;

class disassembler_view_t {
	struct line_t {
		csip_addr_t addr;
		byte        length = 0;
		std::string s;
	};

	float first_line_offset_y = 0;

	std::deque<line_t> lines;

	std::optional<csip_addr_t> focus_addr;

	float line_height;

	disasm_i8086_t *disassembler;

	int instruction_length_at_address(csip_addr_t addr);

	void disassemble_line(line_t &line, csip_addr_t addr);
	void disassemble_previous_line(line_t &line, csip_addr_t addr);

public:
	disassembler_view_t();

	void focus(csip_addr_t addr);
	void draw(const char *title, bool *open, read_cb_t read);
};

#endif
