#ifndef SUPPORT_TYPES_H
#define SUPPORT_TYPES_H

#include <cstddef>
#include <cstdint>

typedef uint8_t byte;
typedef unsigned int uint;

inline
uint16_t readle16(byte *p) {
	return (uint16_t(p[0]) << 0u)
	     + (uint16_t(p[1]) << 8u);
}

inline
uint16_t readbe16(byte *p) {
	return (uint16_t(p[1]) << 0u)
	     + (uint16_t(p[0]) << 8u);
}

inline
uint16_t readle32(byte *p) {
	return (uint32_t(p[0]) <<  0u)
	     + (uint32_t(p[1]) <<  8u)
	     + (uint32_t(p[2]) << 16u)
	     + (uint32_t(p[3]) << 24u);
}

inline
void writele16(byte *p, uint16_t v) {
	p[0] = (v >>  0u) & 0xffu;
	p[1] = (v >>  8u) & 0xffu;
}

inline
void writebe16(byte *p, uint16_t v) {
	p[0] = (v >>  8u) & 0xffu;
	p[1] = (v >>  0u) & 0xffu;
}

inline
void writele32(byte *p, uint32_t v) {
	p[0] = (v >>  0u) & 0xffu;
	p[1] = (v >>  8u) & 0xffu;
	p[2] = (v >> 16u) & 0xffu;
	p[3] = (v >> 24u) & 0xffu;
}

inline
void writebe32(byte *p, uint32_t v) {
	p[0] = (v >> 24u) & 0xffu;
	p[1] = (v >> 16u) & 0xffu;
	p[2] = (v >>  8u) & 0xffu;
	p[3] = (v >>  0u) & 0xffu;
}

inline
byte readlo(uint16_t v) {
	return (byte)v;
}

inline
byte readhi(uint16_t v) {
	return (byte)(v >> 8);
}

inline
void writelo(uint16_t &dst, byte b) {
	dst = (dst & 0xff00) | b;
}

inline
void writehi(uint16_t &dst, byte b) {
	dst = (dst & 0x00ff) | (uint16_t(b) << 8);
}

#endif
