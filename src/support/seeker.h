#ifndef SUPPORT_SEEKER_H
#define SUPPORT_SEEKER_H

#include "support/types.h"

class seeker_t {
public:
	virtual int seek_set(size_t offset) = 0;
};

#endif
