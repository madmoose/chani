#include "names.h"

void names_t::add_name(uint32_t addr, const std::string name) {
	m_names[addr] = name;
}

const std::string names_t::get_name(uint32_t addr, int *offset) {
	auto it = m_names.lower_bound(addr);

	if (offset) {
		*offset = addr - it->first;
	}

	return it->second;
}
