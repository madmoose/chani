#ifndef EMU_DEVICE_H
#define EMU_DEVICE_H

#include "support/types.h"

class ibm5160_t;

class device_t {
protected:
	ibm5160_t *machine;

public:
	void set_machine(ibm5160_t *a_machine) {
		machine = a_machine;
	}

	virtual double   frequency_in_mhz() = 0;
	virtual uint64_t next_cycles() = 0;
	virtual uint64_t run_cycles(uint64_t cycles) = 0;
};

#endif
