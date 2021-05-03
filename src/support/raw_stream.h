#ifndef RAW_STREAM_H
#define RAW_STREAM_H

#include "types.h"
#include "endian.h"

#include <string.h>
#include <string>

/*
 * This header defines the following classes:
 *
 * raw_istream_t  - a virtual base class for input streams
 * raw_ifstream_t - a class for reading from files
 * raw_imstream_t - a class for reading from a piece of memory
 *
 * raw_ostream_t  - a virtual base class for output streams
 * raw_omstream_t - a class for writing to a piece of memory
 *
 * For reading from a file, it's probably preferable to read the entire file
 * into memory using raw_imstream_t - there's a constructor that takes a filename.
 *
 */

class raw_istream_t
{
protected:
	size_t _size;
	size_t _pos;
	bool   _good;

	std::string _name;

public:
	raw_istream_t()
		: _size(0), _pos(0), _good(true)
	{}
 	virtual ~raw_istream_t()
	{}

	virtual void read(byte *p, size_t s) = 0;

	byte   readbyte();

	uint16_t readle16();
	uint16_t readbe16();

	uint32_t readle32();
	uint32_t readbe32();

	uint32_t readbe32at(size_t ofs);

	uint64_t readle64();

	uint16_t readaheadbe16();

	size_t size();
	size_t pos();
	size_t rem();
	bool   good();

	virtual void seek_set(size_t p);
	void seek_cur(int d);

	virtual void reset() = 0;

	void set_name(std::string a_name) { _name = a_name; }
	const std::string name() { return _name; }
private:
	raw_istream_t(const raw_istream_t&);
	const raw_istream_t& operator=(const raw_istream_t&);
};

class raw_ifstream_t : public raw_istream_t
{
	int _fd;
public:
	raw_ifstream_t(const std::string &filename);
	~raw_ifstream_t();

	void read(byte *p, size_t s);
	void seek_set(size_t p);
	void reset();
};

#define DELETE_WHEN_DONE true

class raw_imstream_t : public raw_istream_t
{
	byte *_p;
	bool  _delete_when_done;

public:
	raw_imstream_t(byte *p, size_t s, bool delete_when_done = false);
	raw_imstream_t(const std::string &filename);
	~raw_imstream_t();

	void read(byte *p, size_t s);
	void reset();

private:
	raw_imstream_t(const raw_imstream_t&);
	const raw_imstream_t& operator=(const raw_imstream_t&);
};

class raw_ostream_t
{
protected:
	size_t _size;
	size_t _pos;
	bool   _good;

public:
	raw_ostream_t()
		: _size(0), _pos(0), _good(true)
	{}
	virtual ~raw_ostream_t()
	{}

	virtual void write(const byte *p, size_t s) = 0;

	void writebyte(byte a);

	void writele16(uint16_t a);
	void writebe16(uint16_t a);

	void writele32(uint32_t a);
	void writebe32(uint32_t a);

	size_t size();
	size_t pos();
	size_t rem();
	bool   good();

	void seek_set(size_t p);
	void seek_cur(int  d);
};

class raw_omstream_t : public raw_ostream_t
{
protected:
	byte   *_p;
	bool    _delete_when_done;
	bool    _expanding;
	size_t  _capacity;

public:
	raw_omstream_t(byte *p, size_t s, bool delete_when_done = false);
	raw_omstream_t(size_t size);
	raw_omstream_t();
	~raw_omstream_t();

	void write(const byte *p, size_t s);

private:
	raw_omstream_t(const raw_omstream_t&);
	const raw_omstream_t& operator=(const raw_omstream_t&);
};

/*
 * raw_istream_t inline methods
 */

inline
byte raw_istream_t::readbyte()
{
	byte a[1];
	read(a, sizeof(a));
	return a[0];
}

inline
uint16_t raw_istream_t::readle16()
{
	byte a[2];
	read(a, sizeof(a));
	return ::readle16(a);
}

inline
uint16_t raw_istream_t::readbe16()
{
	byte a[2];
	read(a, sizeof(a));
	return ::readbe16(a);
}

inline
uint32_t raw_istream_t::readle32()
{
	byte a[4];
	read(a, sizeof(uint32_t));
	return ::readle32(a);
}

inline
uint32_t raw_istream_t::readbe32()
{
	byte a[4];
	read(a, sizeof(a));
	return ::readbe32(a);
}

inline
uint64_t raw_istream_t::readle64()
{
	byte a[8];
	read(a, sizeof(a));
	return ::readle64(a);
}

inline
uint32_t raw_istream_t::readbe32at(size_t ofs)
{
	byte a[4];
	seek_set(ofs);
	read(a, sizeof(a));
	return ::readbe32(a);
}

inline
uint16_t raw_istream_t::readaheadbe16()
{
	uint16_t a = readbe16();
	seek_cur(-2);
	return a;
}

inline
size_t raw_istream_t::size()
{
	return _size;
}

inline
size_t raw_istream_t::pos()
{
	return _pos;
}

inline
size_t raw_istream_t::rem()
{
	return size() - pos();
}

inline
bool raw_istream_t::good()
{
	return _good;
}

inline
void raw_istream_t::seek_set(size_t p)
{
	if (p > size()) {
		_good = false;
		return;
	}

	_pos = p;
}

inline
void raw_istream_t::seek_cur(int d)
{
	seek_set(pos() + d);
}

/*
 * raw_imstream_t inline classes
 */

inline
void raw_imstream_t::read(byte *p, size_t s)
{
	if (!_good)
		return;

	if (s > rem()) {
		_good = false;
		return;
	}

	memcpy(p, _p + _pos, s);
	_pos += s;
}

/*
 * raw_ostream_t inline methods
 */

inline
size_t raw_ostream_t::size()
{
	return _size;
}

inline
size_t raw_ostream_t::pos()
{
	return _pos;
}

inline
size_t raw_ostream_t::rem()
{
	return size() - pos();
}

inline
bool raw_ostream_t::good()
{
	return _good;
}

inline
void raw_ostream_t::seek_set(size_t p)
{
	_pos = p;
}

inline
void raw_ostream_t::seek_cur(int d)
{
	seek_set(pos() + d);
}

#endif
