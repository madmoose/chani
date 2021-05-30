#ifndef SUPPORT_WRITER_H
#define SUPPORT_WRITER_H

#include "support/types.h"

class writer_t {
public:
	virtual size_t write(byte *b, size_t size) = 0;

	void writebyte(byte b) {
		write(&b, 1);
	}

	void writele16(uint16_t v) {
		byte b[2];
		::writele16(b, v);
		write(b, 2);
	}

	void writebe16(uint16_t v) {
		byte b[2];
		::writebe16(b, v);
		write(b, 2);
	}

	void writele32(uint32_t v) {
		byte b[4];
		::writele32(b, v);
		write(b, 4);
	}

	void writebe32(uint32_t v) {
		byte b[4];
		::writebe32(b, v);
		write(b, 4);
	}
};

#endif
