#include "disassembler_view.h"

#include "disasm/disasm_i8086.h"
#include "support/reader.h"

#include <bit>
#include <cassert>
#include <cmath>
#include <cstdio>

#include <imgui.h>

static uint32_t fnv_1a_hash(reader_t &r, int len) {
	assert(len > 0 && len < 256);

	uint32_t hash = 0x811c9dc5u;
	while (len--) {
		hash *= 0x01000193u;
		hash ^= r.readbyte();
	}
	return hash;
}

// The smallest integral power of two that is not smaller than x.
// Defined as std::bit_ceil in c++20
static uint32_t bit_ceil(uint32_t x) {
	uint32_t r = 1;
	while (r < x) {
		r *= 2;
	}
	return r;
}

disassembler_view_t::disassembler_view_t() {
	disassembler = new disasm_i8086_t;
}

void disassembler_view_t::focus(csip_addr_t addr) {
	focus_addr = addr;
}

void disassembler_view_t::disassemble_line(line_t &line, csip_addr_t addr) {
	if (line.addr.ea() == addr.ea() && line.length > 0) { // TODO: Check byte range hash
		return;
	}

	uint16_t ip = addr.ip;
	const char *s;
	disassembler->disassemble(addr.cs, &ip, &s);

	line.addr = addr;
	line.length = ip - addr.ip;
	line.s = s;
}

int disassembler_view_t::instruction_length_at_address(csip_addr_t addr) {
	uint16_t ip = addr.ip;
	disassembler->disassemble(addr.cs, &ip, nullptr);
	return ip - addr.ip;
}

void disassembler_view_t::disassemble_previous_line(line_t &line, csip_addr_t addr) {
	assert(addr.ea() != 0);

	/*
	 * Scan backwards until we find an instruction that
	 * doesn't overlap with the current instruction address.
	 */
	csip_addr_t previous_addr = addr;
	int instruction_length = 0;
	do {
		if (previous_addr.ip == 0) {
			break;
		}
		--previous_addr;
		instruction_length = instruction_length_at_address(previous_addr);
	} while (previous_addr.ea() + instruction_length > addr.ea());

	/*
	 * If there's a gap between the two instructions, output
	 * the first previous byte as a literal.
	 */
	if (previous_addr.ea() + instruction_length != addr.ea()) {
		previous_addr = addr;
		--previous_addr;
		line.addr = previous_addr;

		strbuf_t sb;
		sb.sprintf("db %02x", disassembler->read(MEM, previous_addr.ea(), W8));
		line.s = sb.cstr();
		return;
	}

	uint16_t ip = previous_addr.ip;
	const char *s;
	disassembler->disassemble(addr.cs, &ip, &s);
	line.addr = previous_addr;
	line.length = ip - previous_addr.ip;
	line.s = s;
}

void disassembler_view_t::draw(const char *title, bool *open, read_cb_t read) {
	ImGui::SetNextWindowSize(ImVec2(520,600), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin(title, open)) {
		ImGui::End();
		return;
	}

	if (!ImGui::BeginChild("disasm")) {
		ImGui::EndChild();
		return;
	}

	disassembler->read = read;

	ImGuiContext &g = *ImGui::GetCurrentContext();

	float window_height = ImGui::GetWindowHeight();
	float line_height = ImGui::GetTextLineHeight() + 2;

	// first_line_offset_y is zero or negative, subtracting increases the value.
	int visible_line_count = ceil((window_height - first_line_offset_y) / line_height);

	if (lines.size() < visible_line_count) {
		lines.resize(bit_ceil(visible_line_count));
	}

	uint        anchor_line_index = 0;
	float       anchor_line_offset_y;
	csip_addr_t anchor_line_addr;

	if (focus_addr.has_value()) {
		anchor_line_addr = focus_addr.value();
		focus_addr.reset();
		anchor_line_offset_y = 0; // round((window_height - line_height) / 2.0f);
	} else {
		anchor_line_offset_y = first_line_offset_y;
		anchor_line_index = 0;
		anchor_line_addr = lines[0].addr;
	}

	float wheel_delta = ImGui::GetIO().MouseWheel;
	float scroll_step = floor(5 * ImGui::GetFontSize());

	anchor_line_offset_y += scroll_step * wheel_delta;

	disassemble_line(lines[anchor_line_index], anchor_line_addr);

	// Traverse backwards from anchor line to first visible line
	{
		int         line_index    = anchor_line_index;
		float       line_offset_y = anchor_line_offset_y;
		csip_addr_t line_addr     = anchor_line_addr;

		while (line_offset_y > 0.0f && line_addr.ip > 0) {
			lines.push_front(line_t{});
			disassemble_previous_line(lines[0], line_addr);

			line_offset_y -= line_height;
			line_addr = lines[0].addr;
		}

		first_line_offset_y = line_offset_y;
	}

	if (first_line_offset_y > 0.0f) {
		anchor_line_offset_y -= first_line_offset_y;
		first_line_offset_y = 0.0;
	}

	// Traverse forwards from anchor line to last visible line
	{
		int         line_index    = anchor_line_index;
		float       line_offset_y = anchor_line_offset_y;
		csip_addr_t line_addr     = anchor_line_addr;
		int         inst_len      = lines[line_index].length;

		for (;;) {
			line_index    = line_index + 1;
			line_offset_y = line_offset_y + line_height;
			line_addr.ip += inst_len;

			if (line_offset_y > window_height) {
				break;
			}

			if (lines.size() <= line_index) {
				lines.push_back(line_t{});
			}

			disassemble_line(lines[line_index], line_addr);
			inst_len = lines[line_index].length;

		}
	}

	// Trim excessive lines at the top
	while (first_line_offset_y <= -line_height) {
		lines.pop_front();
		first_line_offset_y += line_height;
	}

	// Trim excessive lines at the bottom
	lines.resize(visible_line_count);



	ImDrawList *drawlist = ImGui::GetWindowDrawList();

	auto cursor = ImGui::GetCursorScreenPos();
	cursor.y += first_line_offset_y;
	for (int i = 0; i != visible_line_count; ++i) {
		std::string &s = lines[i].s;
		drawlist->AddText(cursor, ImGui::GetColorU32(ImGuiCol_Text), s.c_str(), s.c_str() + s.length());
		cursor.y += line_height;
	}

	ImGui::EndChild();
	ImGui::End();
}
