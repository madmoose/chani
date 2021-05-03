#ifndef ENDIAN_H
#define ENDIAN_H

#include "types.h"

#ifndef BIG_ENDIAN
#define BIG_ENDIAN     0x1234
#endif

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN  0x4321
#endif

#ifndef BYTE_ORDER
#define BYTE_ORDER     LITTLE_ENDIAN
#endif

#if BYTE_ORDER == LITTLE_ENDIAN
#define ENDIAN_VALUE_LE_BE(le, be) (le)
#else
#define ENDIAN_VALUE_LE_BE(le, be) (be)
#endif

inline
uint16_t swap16(uint16_t a)
{
	return ((a >> 8u) & 0x00FF) |
	       ((a << 8u) & 0xFF00);
}

inline
uint32_t swap32(uint32_t a)
{
	return ((a >> 24u) & 0x000000FF) |
	       ((a >>  8u) & 0x0000FF00) |
	       ((a <<  8u) & 0x00FF0000) |
	       ((a << 24u) & 0xFF000000);
}

#if BYTE_ORDER == BIG_ENDIAN
inline
uint16_t letoh16(uint16_t a)
{
	return swap16(a);
}

inline
uint16_t betoh16(uint16_t a)
{
	return a;
}

inline
uint32_t letoh32(uint32_t a)
{
	return swap32(a);
}

inline
uint32_t betoh32(uint32_t a)
{
	return a;
}

inline
uint16_t htole16(uint16_t a)
{
	return swap16(a);
}

inline
uint16_t htobe16(uint16_t a)
{
	return a;
}

inline
uint32_t htole32(uint32_t a)
{
	return swap32(a);
}

inline
uint32_t htobe32(uint32_t a)
{
	return a;
}
#elif BYTE_ORDER == LITTLE_ENDIAN
inline
uint16_t letoh16(uint16_t a)
{
	return a;
}

inline
uint16_t betoh16(uint16_t a)
{
	return swap16(a);
}

inline
uint32_t letoh32(uint32_t a)
{
	return a;
}

inline
uint32_t betoh32(uint32_t a)
{
	return swap32(a);
}

#ifndef htole16
inline
uint16_t htole16(uint16_t a)
{
	return a;
}
#endif

#ifndef htobe16
inline
uint16_t htobe16(uint16_t a)
{
	return swap16(a);
}
#endif

#ifndef htole32
inline
uint32_t htole32(uint32_t a)
{
	return a;
}
#endif

#ifndef htobe32
inline
uint32_t htobe32(uint32_t a)
{
	return swap32(a);
}
#endif

#else
#error Unknown endianess!
#endif

inline
uint16_t readle16(byte *p)
{
	return (uint16_t(p[0]) << 0u)
	     + (uint16_t(p[1]) << 8u);
}

inline
uint16_t readbe16(byte *p)
{
	return (uint16_t(p[1]) << 0u)
	     + (uint16_t(p[0]) << 8u);
}

inline
uint32_t readle32(byte *p)
{
	return (uint32_t(p[0]) <<  0u)
	     + (uint32_t(p[1]) <<  8u)
	     + (uint32_t(p[2]) << 16u)
	     + (uint32_t(p[3]) << 24u);
}

inline
uint32_t readbe32(byte *p)
{
	return (uint32_t(p[3]) <<  0u)
	     + (uint32_t(p[2]) <<  8u)
	     + (uint32_t(p[1]) << 16u)
	     + (uint32_t(p[0]) << 24u);
}

inline
uint64_t readle48(byte *p)
{
	return (uint64_t(p[0]) <<  0u)
	     + (uint64_t(p[1]) <<  8u)
	     + (uint64_t(p[2]) << 16u)
	     + (uint64_t(p[3]) << 24u)
	     + (uint64_t(p[4]) << 32u)
	     + (uint64_t(p[5]) << 40u);
}

inline
uint64_t readle64(byte *p)
{
	return (uint64_t(p[0]) <<  0u)
	     + (uint64_t(p[1]) <<  8u)
	     + (uint64_t(p[2]) << 16u)
	     + (uint64_t(p[3]) << 24u)
	     + (uint64_t(p[4]) << 32u)
	     + (uint64_t(p[5]) << 40u)
	     + (uint64_t(p[6]) << 48u)
	     + (uint64_t(p[7]) << 56u);
}

inline
void writele16(byte *p, uint16_t a)
{
	p[0] = (a >>  0u) & 0xffu;
	p[1] = (a >>  8u) & 0xffu;
}

inline
void writebe16(byte *p, uint16_t a)
{
	p[1] = (a >>  0u) & 0xffu;
	p[0] = (a >>  8u) & 0xffu;
}

inline
void writele32(byte *p, uint32_t a)
{
	p[0] = (a >>  0u) & 0xffu;
	p[1] = (a >>  8u) & 0xffu;
	p[2] = (a >> 16u) & 0xffu;
	p[3] = (a >> 24u) & 0xffu;
}

inline
void writebe32(byte *p, uint32_t a)
{
	p[3] = (a >>  0u) & 0xffu;
	p[2] = (a >>  8u) & 0xffu;
	p[1] = (a >> 16u) & 0xffu;
	p[0] = (a >> 24u) & 0xffu;
}

inline
void writele64(byte *p, uint64_t a)
{
	p[0] = (a >>  0u) & 0xffu;
	p[1] = (a >>  8u) & 0xffu;
	p[2] = (a >> 16u) & 0xffu;
	p[3] = (a >> 24u) & 0xffu;
	p[4] = (a >> 32u) & 0xffu;
	p[5] = (a >> 40u) & 0xffu;
	p[6] = (a >> 48u) & 0xffu;
	p[7] = (a >> 56u) & 0xffu;
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
