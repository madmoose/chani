#ifndef SUPPORT_MEM_WRITER_H
#define SUPPORT_MEM_WRITER_H

#include "support/writer.h"
#include "support/seeker.h"

#include <cassert>
#include <cstring>

class mem_writer_t : public writer_t, public seeker_t {
	byte   *p;
	size_t  offset;
	size_t  size;

public:
	mem_writer_t(byte *p, size_t size)
		: p(p), offset(0), size(size)
	{}

	size_t len() { return offset; }

	size_t write(byte *b, size_t write_size) {
		assert(write_size < size - offset);
		memmove(p + offset, b, write_size);
		offset += write_size;
		return write_size;
	}

	int seek_set(size_t new_offset) {
		offset += new_offset;
		return 0;
	}
};

#endif
