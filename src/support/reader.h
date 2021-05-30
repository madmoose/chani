#ifndef SUPPORT_READER_H
#define SUPPORT_READER_H

#include "support/types.h"

class reader_t {
public:
	virtual size_t read(byte *p, size_t s) = 0;

	byte readbyte() {
		byte b;
		read(&b, 1);
		return b;
	}

	uint16_t readle16() {
		byte b[2];
		read(b, 2);
		return ::readle16(b);
	}

	uint16_t readbe16() {
		byte b[2];
		read(b, 2);
		return ::readbe16(b);
	}

	uint16_t readle32() {
		byte b[4];
		read(b, 4);
		return ::readle32(b);
	}
};

#endif
