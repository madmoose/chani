#include "dos/dos.h"
#include "emu/i8086.h"
#include "emu/i8254_pit.h"
#include "emu/ibm5160.h"
#include "emu/vga.h"
#include "gui/machine_runner.h"
#include "gui/main_window.h"
#include "support/file_reader.h"
#include "support/mem_writer.h"

#include <cstdio>
#include <memory>
#include <mutex>
#include <thread>

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

	machine_runner_t *machine_runner = new machine_runner_t(&*machine);

	auto main_window = new main_window_t(machine_runner);

	main_window->initialize_glfw();
	main_window->initialize_imgui();

	main_window->loop();

	main_window->uninitialize_imgui();
	main_window->uninitialize_glfw();

	return 0;
}
