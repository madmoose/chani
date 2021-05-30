#include "support/file_reader.h"

file_reader_t::file_reader_t(std::string path) {
	f = fopen(path.c_str(), "r");
}

file_reader_t::~file_reader_t() {
	fclose(f);
}

bool file_reader_t::eof() {
	return !f || feof(f);
}

int file_reader_t::seek_set(size_t offset) {
	return fseek(f, offset, SEEK_SET);
}

size_t file_reader_t::read(byte *p, size_t s) {
	return fread(p, s, 1, f);
}
