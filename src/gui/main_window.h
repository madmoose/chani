#ifndef GUI_MAIN_WINDOW_H
#define GUI_MAIN_WINDOW_H

struct GLFWwindow;

class machine_runner_t;

class main_window_t {
	GLFWwindow *window;
	machine_runner_t *machine_runner;

public:
	main_window_t(machine_runner_t *machine_runner) :
		machine_runner(machine_runner)
	{}

	void initialize_glfw();
	void initialize_imgui();

	void loop();

	void uninitialize_imgui();
	void uninitialize_glfw();
};

#endif
