#ifndef BINARIES_EXE_MZ_H
#define BINARIES_EXE_MZ_H

#include "exe_mz_header.h"

#include "support/types.h"

#include <string>
#include <vector>

class file_reader_h;

struct exe_mz_relocation_t {
	uint16_t ofs;
	uint16_t seg;
};

struct exe_mz_t {
	~exe_mz_t();

	std::string _name;

	const std::string name() { return _name; }
	const std::string format() { return "DOS MZ executable"; }

	bool load(file_reader_t &rd);

	exe_mz_header_t                  head;
	std::vector<exe_mz_relocation_t> relocations;

	uint32_t  image_size;
	byte     *image;
};

#endif
