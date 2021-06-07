#ifndef EMU_MACHINE_RUNNER_H
#define EMU_MACHINE_RUNNER_H

#include "emu/device.h"

#include <functional>
#include <mutex>
#include <thread>
#include <vector>

class ibm5160_t;

class machine_runner_t {
	struct device_next_event_t {
		const char *name;
		device_t   *device;
		double      next_event;
	};

	std::vector<device_next_event_t> devices;

	std::mutex  machine_mutex;
	ibm5160_t  *machine;

	std::thread      *thread;
	std::atomic_flag  stop_requested;
public:
	machine_runner_t(ibm5160_t *machine);

	void stop();

	void with_machine(const std::function<void(ibm5160_t *)> &f);

	void run_until_next_event();
	void loop();
};

#endif
