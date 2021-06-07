#ifndef SUPPORT_FILE_READER_H
#define SUPPORT_FILE_READER_H

#include "support/reader.h"

#include <cstdio>
#include <string>

class file_reader_t : public reader_t {
	FILE *f;

public:
	file_reader_t(std::string path);
	~file_reader_t();

	bool eof();

	int seek_set(size_t offset);
	size_t read(byte *p, size_t s);
};

#endif
