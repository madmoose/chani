#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef uint8_t      byte;
typedef unsigned int uint;

inline
uint8_t hi(uint16_t v)
{
	return uint8_t(v >> 8u);
}

inline
uint8_t lo(uint16_t v)
{
	return uint8_t(v & 0xffu);
}

inline
uint16_t concat(uint8_t a, uint8_t b) {
	return (uint16_t(a) << 8u)
	     | (uint16_t(b) << 0u);
}

inline
uint32_t concat(uint8_t a, uint8_t b, uint8_t c) {
	return (uint32_t(a) << 16u)
	     | (uint32_t(b) <<  8u)
	     | (uint32_t(c) <<  0u);
}

inline
uint32_t concat(uint16_t a, uint16_t b) {
	return (uint32_t(a) << 16u)
	     | (uint32_t(b) <<  0u);
}

#endif
