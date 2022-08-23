#include "support/strbuf.h"

#include <cstdarg>
#include <cstdlib>
#include <cstdio>

void strbuf_t::ensure_cap(int n) {
	if (n <= cap) {
		return;
	}

	int new_cap = cap ? cap : 64;
	while (new_cap <= n)  {
		new_cap *= 2;
	}
	char *ptr = (char*)realloc(s, new_cap);
	if (!ptr) {
		fprintf(stderr, "realloc error");
		exit(-1);
	}
	s = ptr;
	cap = new_cap;
}

int strbuf_t::sprintf(const char *format, ...) {
	int n;
	{
		va_list ap;
		va_start(ap, format);
		n = vsnprintf(nullptr, 0, format, ap);
		va_end(ap);
	}

	ensure_cap(len + n + 1);

	{
		va_list ap;
		va_start(ap, format);
		vsprintf(s+len, format, ap);
		va_end(ap);
	}

	len += n;

	return len;
}

void strbuf_t::align_col(int col) {
	while (len < col) {
		sprintf(" ");
	}
}
