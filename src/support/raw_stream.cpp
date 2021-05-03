#include "raw_stream.h"

#include <cassert>
#include <fcntl.h>
#include <sys/types.h>
#ifndef _WIN32
#include <sys/uio.h>
#include <unistd.h>
#else
#include <io.h>
#endif

#include <algorithm>

/*
 * raw_ifstream_t
 */

raw_ifstream_t::raw_ifstream_t(const std::string &filename)
{
	_name = filename;
#ifdef _WIN32
	_fd = _open(filename.c_str(), _O_RDONLY | _O_BINARY);
#else
	_fd = open(filename.c_str(), O_RDONLY);
#endif
	if (_fd < 0) {
		_good = false;
		return;
	}
#ifdef _WIN32
	__int64 size = _lseeki64(_fd, 0, SEEK_END);
	assert(size >= 0);
	__int64 pos  = _lseeki64(_fd, 0, SEEK_SET);
	assert(pos >= 0);
	_size = size_t(size);
	_pos = size_t(pos);
#else
	_size = lseek(_fd, 0, SEEK_END);
	_pos = lseek(_fd, 0, SEEK_SET);
#endif
}

raw_ifstream_t::~raw_ifstream_t()
{
	if (_fd >= 0)
#ifdef _WIN32
		_close(_fd);
#else
		close(_fd);
#endif
}

void raw_ifstream_t::read(byte *p, size_t s)
{
#ifdef _WIN32
	int r;
#else
	ssize_t r;
#endif

	if (!_good)
		return;

	do {
#ifdef _WIN32
		assert(s <= UINT_MAX);
		r = _read(_fd, p, (uint)s);
#else
		r = ::read(_fd, p, s);
#endif
		if (r <= 0) {
			_good = false;
			return;
		}

		_pos += r;
		p += r;
		s -= r;
	} while (s);
}

void raw_ifstream_t::seek_set(size_t p)
{
	if (!_good)
		return;

	if (p > size()) {
		_pos = size();
		_good = false;
	}

#ifdef _WIN32
	__int64 pos = _lseeki64(_fd, p, SEEK_SET);
#else
	off_t pos = lseek(_fd, p, SEEK_SET);
#endif
	assert(pos >= 0);
	_pos = size_t(pos);
}

inline
void raw_ifstream_t::reset()
{
	if (_fd >= 0) {
		_good = true;
		seek_set(0);
	}
}

/*
 * raw_imstream_t
 */

raw_imstream_t::raw_imstream_t(byte *p, size_t s, bool delete_when_done)
{
	_p = p;
	_size = s;
	_pos = 0;
	_delete_when_done = delete_when_done;
}

raw_imstream_t::raw_imstream_t(const std::string &filename)
	: _p(0), _delete_when_done(false)
{
	raw_ifstream_t is(filename);
	if (!is.good()) {
		_good = false;
		return;
	}
	_size = is.size();
	_p = new byte[_size];
	is.read(_p, _size);
	_pos = 0;
	_delete_when_done = true;
}

raw_imstream_t::~raw_imstream_t()
{
	if (_delete_when_done)
		delete[] _p;
}

void raw_imstream_t::reset()
{
	if (_p) {
		_good = true;
		_pos = 0;
	}
}

/*
 * raw_ostream_t
 */

void raw_ostream_t::writebyte(byte a)
{
	write(&a, sizeof(a));
}

void raw_ostream_t::writele16(uint16_t a)
{
	a = htole16(a);
	write((byte*)&a, sizeof(a));
}

void raw_ostream_t::writebe16(uint16_t a)
{
	a = htobe16(a);
	write((byte*)&a, sizeof(a));
}

void raw_ostream_t::writele32(uint32_t a)
{
	a = htole32(a);
	write((byte*)&a, sizeof(a));
}

void raw_ostream_t::writebe32(uint32_t a)
{
	a = htobe32(a);
	write((byte*)&a, sizeof(a));
}

/*
 * raw_omstream_t
 */

raw_omstream_t::raw_omstream_t(byte *p, size_t size, bool delete_when_done)
{
	_p = p;
	_pos = 0;
	_size = size;
	_delete_when_done = delete_when_done;
	_expanding = false;
}

raw_omstream_t::raw_omstream_t(size_t size)
{
	_p = new byte[size];
	_pos = 0;
	_size = size;
	_delete_when_done = true;
	_expanding = false;
}

raw_omstream_t::raw_omstream_t()
{
	_p = 0;
	_pos = 0;
	_size = 0;
	_delete_when_done = true;
	_expanding = true;
	_capacity = 0;
}

raw_omstream_t::~raw_omstream_t()
{
	if (_delete_when_done)
		delete[] _p;
}

void raw_omstream_t::write(const byte *p, size_t s)
{
	if (_expanding && _pos + s >= _capacity) {
		assert(_capacity <= SIZE_MAX / 2);
		size_t new_capacity = 2 * _capacity;
		while (_pos + s >= new_capacity)
			new_capacity *= 2;
		byte *new_p = new byte[new_capacity];
		memcpy(new_p, _p, _size);
		delete[] _p;
		_p = new_p;
		_capacity = new_capacity;
		_size = std::max(_size, _pos + s);
	}

	memcpy(_p + _pos, p, s);
	_pos += s;
}
