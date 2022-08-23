#ifndef DISASM_NAMES
#define DISASM_NAMES

#include <map>
#include <string>

#include <iostream>
#include <cassert>

class names_t {
	std::map<uint32_t, std::string, std::greater<uint32_t>> m_names;

public:
	names_t() {
		m_names[0] = "---";
	}

	void add_name(uint32_t addr, const std::string name);
	const std::string get_name(uint32_t addr, int *offset = nullptr);
};

#endif
