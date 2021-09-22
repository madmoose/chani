#include "gui/machine_runner.h"

#include "dos/dos.h"
#include "emu/device.h"
#include "emu/i8086.h"
#include "emu/i8254_pit.h"
#include "emu/ibm5160.h"
#include "emu/vga.h"
#include "emu/keyboard.h"

#include <thread>
#include <vector>

machine_runner_t::machine_runner_t(ibm5160_t *machine) :
	machine(machine)
{
	for (const auto &registered_device : machine->devices) {
		devices.push_back({
			device_next_event_t {
				registered_device.name,
				registered_device.device,
				0.0
			}
		});
	}

	thread = new std::thread(&machine_runner_t::loop, this);
}

void machine_runner_t::stop() {
	{
		std::lock_guard<std::mutex> lock(state_mutex);
		state = MACHINE_RUNNER_STATE_STOP;
	}
	state_cv.notify_one();
	thread->join();
}

void machine_runner_t::pause() {
	state = MACHINE_RUNNER_STATE_PAUSE;
}

void machine_runner_t::resume() {
	{
		std::lock_guard<std::mutex> lock(state_mutex);
		state = MACHINE_RUNNER_STATE_RUN;
	}
	state_cv.notify_one();
}

void machine_runner_t::with_machine(const std::function<void(ibm5160_t *)> &f) {
	std::lock_guard<std::mutex> lock(machine_mutex);
	f(machine);
}

void machine_runner_t::set_mouse(uint16_t x, uint16_t y, uint16_t buttons) {
	if (old_mouse_x == x && old_mouse_y == y && old_mouse_buttons == buttons) {
		return;
	}

	old_mouse_x = x;
	old_mouse_y = y;
	old_mouse_buttons = buttons;

	std::lock_guard<std::mutex> lock(machine_mutex);
	machine->dos->set_mouse(x, y, buttons);
}

void machine_runner_t::set_key_down(int down_key_id) {
	std::lock_guard<std::mutex> lock(machine_mutex);
	machine->keyboard->set_key_down(down_key_id);
}

void machine_runner_t::set_key_up(int up_key_id) {
	std::lock_guard<std::mutex> lock(machine_mutex);
	machine->keyboard->set_key_up(up_key_id);
}

void machine_runner_t::run_until_next_event() {
	// Find next event for each devices (in microseconds)
	for (auto &d : devices) {
		uint64_t device_cycles = d.device->next_cycles();
		d.next_event = device_cycles / d.device->frequency_in_mhz();
	}

	auto next_event_device = std::min_element(devices.begin(), devices.end(), [](auto a, auto b) {
		return a.next_event < b.next_event;
	});
	double next_event = next_event_device->next_event;

	// Simulate at most 1ms (1000 microseconds) at a time
	next_event = std::min(next_event, 1000.0);

	// Run all devices until next event
	std::lock_guard<std::mutex> lock(machine_mutex);
	for (auto &d : devices) {
		double   device_frequency = d.device->frequency_in_mhz();
		uint64_t device_cycles = next_event * device_frequency;
		d.device->run_cycles(device_cycles);
	}
}

void machine_runner_t::state_run() {
	run_until_next_event();
	if (machine->vga->frame_ready()) {
		// Limit frame rate to 70 fps
		const auto frame_end = frame_start + std::chrono::nanoseconds(1000000000 / 70);
		std::this_thread::sleep_until(frame_end);
		frame_start = frame_end;
	}
}

void machine_runner_t::state_pause() {
	const auto pause_start = std::chrono::steady_clock::now();

	{
		auto ul = std::unique_lock<std::mutex>(state_mutex);
		state_cv.wait(ul, [this](){ return state != MACHINE_RUNNER_STATE_PAUSE; });
	}

	const auto pause_end = std::chrono::steady_clock::now();
	frame_start += (pause_end - pause_start);
}

void machine_runner_t::loop() {
	frame_start = std::chrono::steady_clock::now();

	for (;;) {
		switch (state) {
			case MACHINE_RUNNER_STATE_RUN:
				state_run();
				break;

			case MACHINE_RUNNER_STATE_PAUSE:
				state_pause();
				break;

			case MACHINE_RUNNER_STATE_STOP:
				return;
		}
	}
}
