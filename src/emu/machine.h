#ifndef EMU_MACHINE
#define EMU_MACHINE

#include "emu/cpu_device.h"
#include "support/types.h"

#include <string>
#include <vector>

class device_t;

struct named_device_t {
	std::string  name;
	device_t    *device;
};

typedef std::vector<named_device_t> devices_t;

class machine_t {
protected:
	device_t *add_plain_device(const char *name, device_t *device);

	template<typename T> T *add_device(const char *name, T *device) {
		add_plain_device(name, device);
		return device;
	}

public:
	devices_t devices;

	cpu_device_t *cpu;
	byte         *memory;

	void raise_nmi() {
		cpu->raise_nmi();
	}

	void raise_intr(byte num) {
		cpu->raise_intr(num);
	}
};

#endif
