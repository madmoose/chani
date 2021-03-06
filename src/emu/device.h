#ifndef EMU_DEVICE_H
#define EMU_DEVICE_H

#include "support/types.h"

class machine_t;

class device_t {
protected:
	machine_t *machine;

public:
	void set_machine(machine_t *a_machine) {
		machine = a_machine;
	}

	virtual double   frequency_in_mhz() = 0;
	virtual uint64_t next_cycles() = 0;
	virtual uint64_t run_cycles(uint64_t cycles) = 0;
};

#endif
