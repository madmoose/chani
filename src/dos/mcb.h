#include "support/raw_stream.h"

class mcb_t {
	byte type;
	byte owner;
	byte size;
	byte unused[3];
	byte dos4[8];
};
