#include "emu/machine.h"

#include "emu/device.h"

device_t *machine_t::add_plain_device(const char *name, device_t *device) {
	devices.push_back(named_device_t{
		name,
		device
	});
	device->set_machine(this);
	return device;
}
