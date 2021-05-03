#ifndef EXE_MZ_H
#define EXE_MZ_H

#include "support/types.h"
#include "exe_mz_header.h"

#include <string>
#include <vector>

struct exe_mz_relocation_t {
	uint16_t ofs;
	uint16_t seg;
};

struct exe_mz_t {
	~exe_mz_t();

	std::string _name;

	const std::string name() { return _name; }
	const std::string format() { return "DOS MZ executable"; }

	bool load(raw_istream_t &is);

	exe_mz_header_t                  head;
	std::vector<exe_mz_relocation_t> relocations;

	uint32_t  image_size;
	byte     *image;
};

#endif
