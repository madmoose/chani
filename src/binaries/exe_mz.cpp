#include "exe_mz.h"

#include "support/raw_stream.h"

#include <stdio.h>

exe_mz_t::~exe_mz_t()
{
	delete[] image;
}

bool exe_mz_t::load(raw_istream_t &is)
{
	printf("exe_mz_t::load\n");
	if (!head.load(is)) {
		puts("Head load failed.");
		return false;
	}

	image_size = 512 * head.e_cp - 16 * head.e_cparhdr;
	if (head.e_cblp)
		image_size += head.e_cblp - 512;

	_name = is.name();

	image = new byte[image_size];

	is.seek_set(16 * head.e_cparhdr);
	is.read(image, image_size);

	// Read relocations
	relocations.reserve(head.e_crlc);
	is.seek_set(head.e_lfarlc);
	for (int i = 0; i != head.e_crlc; ++i) {
		exe_mz_relocation_t reloc;

		reloc.ofs = is.readle16();
		reloc.seg = is.readle16();

		relocations.push_back(reloc);
	}

	printf("is.good(): %d\n", is.good());
	return is.good();
}
