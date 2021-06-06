#ifndef GUI_TEXTURE_H
#define GUI_TEXTURE_H

#include <GL/gl3w.h>

#include "support/types.h"

class texture_t {
	GLuint texture_id;
	int w;
	int h;
	byte *_data;

public:
	texture_t(int w, int h);

	int   id()     { return texture_id; }
	int   width()  { return w; }
	int   height() { return h; }
	byte *data()   { return _data; }

	void  apply();
};

#endif
