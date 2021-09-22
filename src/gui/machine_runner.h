#ifndef EMU_MACHINE_RUNNER_H
#define EMU_MACHINE_RUNNER_H

#include "emu/device.h"

#include <chrono>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class ibm5160_t;

enum {
	MACHINE_RUNNER_STATE_RUN,
	MACHINE_RUNNER_STATE_STOP,
	MACHINE_RUNNER_STATE_PAUSE,
};

class machine_runner_t {
	struct device_next_event_t {
		std::string name;
		device_t   *device;
		double      next_event;
	};

	uint16_t old_mouse_x = -1;
	uint16_t old_mouse_y = -1;
	uint16_t old_mouse_buttons = -1;

	std::chrono::time_point<std::chrono::steady_clock> frame_start;

	std::vector<device_next_event_t> devices;

	std::mutex  machine_mutex;
	ibm5160_t  *machine;

	std::thread      *thread;

	std::condition_variable state_cv;
	std::mutex              state_mutex;
	std::atomic_int         state = MACHINE_RUNNER_STATE_RUN;

	void loop();
	void run_until_next_event();

	void state_run();
	void state_pause();

public:
	machine_runner_t(ibm5160_t *machine);

	void stop();
	void pause();
	void resume();

	void with_machine(const std::function<void(ibm5160_t *)> &f);

	void set_mouse(uint16_t x, uint16_t y, uint16_t buttons);
	void set_key_down(int down_key_id);
	void set_key_up(int up_key_id);
};

#endif
