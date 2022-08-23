#ifndef EMU_I8086_ADDR
#define EMU_I8086_ADDR

#include "support/types.h"

struct i8086_addr_t {
	uint16_t seg;
	uint16_t ofs;
};

inline
i8086_addr_t operator+(const i8086_addr_t addr, int ofs)
{
	return {
		.seg = addr.seg,
		.ofs = uint16_t(addr.ofs + ofs)
	};
}

inline
i8086_addr_t readaddr(byte *p)
{
	return {
		.seg = readle16(p + 2),
		.ofs = readle16(p + 0),
	};
}

inline
void writeaddr(byte *p, i8086_addr_t v) {
	writele16(p + 0, v.ofs);
	writele16(p + 2, v.seg);
}

#endif
