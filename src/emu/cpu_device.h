#ifndef EMU_CPU_DEVICE_H
#define EMU_CPU_DEVICE_H

#include "emu/device.h"

class cpu_device_t : public device_t {
public:
	virtual void raise_nmi()          = 0;
	virtual void raise_intr(byte num) = 0;
};

#endif
