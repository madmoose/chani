#include "dos/dos.h"
#include "emu/i8086.h"
#include "emu/i8254_pit.h"
#include "emu/ibm5160.h"
#include "emu/vga.h"
#include "support/file_reader.h"
#include "support/mem_writer.h"

#include <cstdio>
#include <memory>

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("Usage: %s file\n", argv[0]);
		exit(1);
	}

	auto machine = std::make_unique<ibm5160_t>();

	const char *filename = argv[1];
	file_reader_t exe(filename);
	if (exe.eof()) {
		printf("Unable to open file '%s'\n", filename);
		return -1;
	}
	machine->dos->exec(exe);

	struct device_next_event_t {
		const char *name;
		device_t   *device;
		double      next_event;
	};

	std::vector<device_next_event_t> devices = {
		{ "cpu", machine->cpu, 0.0 },
		{ "vga", machine->vga, 0.0 },
		{ "pit", machine->pit, 0.0 },
	};

	// uint32_t loop = 0;
	for (;;) {
		// printf("\e[2J\e[1;1H%d --\n", ++loop);
		for (auto &d : devices) {
			uint64_t device_cycles = d.device->next_cycles();
			d.next_event = device_cycles / d.device->frequency_in_mhz();
			// printf("next_event: %s %6llu cycles, %11.5f μs\n", d.name, device_cycles, d.next_event);
		}
		// printf("\n");
		std::sort(devices.begin(), devices.end(), [](auto a, auto b) {
			return a.next_event < b.next_event;
		});

		auto next_event_device = std::find_if(devices.begin(), devices.end(), [&](auto a) {
			return a.device != machine->cpu && a.next_event != 0;
		});
		double next_event = next_event_device->next_event;
		// printf("next_event_device: %s, %11.5f μs\n", next_event_device->name, next_event);

#if 0
		for (auto &d : devices) {
			if (d.next_event == 0) {
				continue;
			}
			double   device_frequency = d.device->frequency_in_mhz();
			uint64_t device_cycles = next_event * device_frequency;
			printf("Running %s for %6llu cycles at %6.3f MHz\n", d.name, device_cycles, device_frequency);
		}
#endif

		for (auto &d : devices) {
			if (d.next_event == 0) {
				continue;
			}
			double   device_frequency = d.device->frequency_in_mhz();
			uint64_t device_cycles = next_event * device_frequency;
			d.device->run_cycles(device_cycles);
		}
	}

	return 0;
}
