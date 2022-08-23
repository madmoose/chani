#ifndef SUPPORT_STRBUF_H
#define SUPPORT_STRBUF_H

class strbuf_t {
	int   len;
	int   cap;
	char *s;

	void ensure_cap(int n);

public:
	strbuf_t()
		: len(0), cap(0), s(nullptr)
	{}

	int sprintf(const char *format, ...);

	void clear() {
		len = 0;
	}

	int get_len() {
		return len;
	}

	void align_col(int col);

	const char *cstr() {
		return s;
	}
};

#endif
