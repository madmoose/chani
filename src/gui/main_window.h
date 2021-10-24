#ifndef GUI_MAIN_WINDOW_H
#define GUI_MAIN_WINDOW_H

#include <gui/texture.h>

struct GLFWwindow;

class machine_runner_t;

class main_window_t {
	GLFWwindow *window;
	machine_runner_t *machine_runner;
private:
	void glfw_render_frame();
	void capture_keyboard();
	void create_window_framebuffer(texture_t& frame_texture, int& frame_x, int& frame_y, uint16_t& mouse_btn);
	void create_window_palette_state(byte dac_ram[768]);
	void create_window_debug(int frame_x, int frame_y, const uint16_t& mouse_btn);
	void create_window_hexview();

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
