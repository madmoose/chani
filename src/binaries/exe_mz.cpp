#include "exe_mz.h"

#include <cstdio>

#include "support/file_reader.h"

exe_mz_t::~exe_mz_t()
{
	delete[] image;
}

bool exe_mz_t::load(file_reader_t &rd)
{
	if (!head.load(rd)) {
		puts("Head load failed.");
		return false;
	}

	image_size = 512 * head.e_cp - 16 * head.e_cparhdr;
	if (head.e_cblp)
		image_size += head.e_cblp - 512;

	image = new byte[image_size];

	rd.seek_set(16 * head.e_cparhdr);
	rd.read(image, image_size);

	// Read relocations
	relocations.reserve(head.e_crlc);
	rd.seek_set(head.e_lfarlc);
	for (int i = 0; i != head.e_crlc; ++i) {
		exe_mz_relocation_t reloc;

		reloc.ofs = rd.readle16();
		reloc.seg = rd.readle16();

		relocations.push_back(reloc);
	}

	return !rd.eof();
}
