#include "gui/main_window.h"

#include "emu/ibm5160.h"
#include "emu/vga.h"
#include "gui/machine_runner.h"
#include "gui/texture.h"

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui.h>
#include <imgui_demo.cpp>

#include <cassert>
#include <cstdio>
#include <thread>

void main_window_t::initialize_glfw() {
	glfwSetErrorCallback([](int error, const char *desc) {
		printf("GLFW error %d: %s\n", error, desc);
		exit(-1);
	});

	if (!glfwInit()) {
		printf("Failed to initialize GLFW\n");
		exit(-1);
	}

	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	window = glfwCreateWindow(1280, 1024, "Chani - Simulflow Debugger", nullptr, nullptr);
	assert(window);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (gl3wInit()) {
		printf("Failed to initialize GLFW\n");
		exit(-1);
	}
}

void main_window_t::initialize_imgui() {
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 150");
}

void main_window_t::loop() {
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	texture_t frame_texture(320, 200);

	byte dac_ram[0x300];
	texture_t palette_texture(16, 16);

	// bool show_demo_window = true;

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// ImGui::ShowDemoWindow(&show_demo_window);

		machine_runner->with_machine([&](ibm5160_t *machine) {
			machine->vga->read_rgba(frame_texture.data(), 0xA0000, frame_texture.width(), frame_texture.height());
			machine->vga->read_dac_ram(dac_ram);
		});

		int frame_x = 0;
		int frame_y = 0;
		uint16_t mouse_btn = 0;

		if (ImGui::Begin("Framebuffer")) {
			if (ImGui::BeginChild(123, ImVec2(0, 0), false, ImGuiWindowFlags_NoMove)) {
				ImVec2 wpos = ImGui::GetWindowPos();
				ImVec2 frame_size = ImVec2(2 * frame_texture.width(), 2 * frame_texture.height());

				frame_texture.apply();
				ImGui::Image((ImTextureID)frame_texture.id(), frame_size);

				if (ImGui::IsWindowFocused()) {
					ImVec2 mouse_pos = ImGui::GetMousePos();

					frame_x = 640 * ((mouse_pos.x - wpos.x) / frame_size.x);
					frame_y = 200 * ((mouse_pos.y - wpos.y) / frame_size.y);

					mouse_btn = 0;
					if (frame_x >= 0 && frame_x < 640 && frame_y >= 0 && frame_y < 200) {
						ImGui::SetMouseCursor(ImGuiMouseCursor_None);
						if (ImGui::IsMouseDown(0)) {
							mouse_btn |= 1;
						}
						if (ImGui::IsMouseDown(1)) {
							mouse_btn |= 2;
						}
					}
					machine_runner->set_mouse(frame_x, frame_y, mouse_btn);
				}
			}
			ImGui::EndChildFrame();
		}
		ImGui::CaptureKeyboardFromApp(false);
		ImGuiIO& io = ImGui::GetIO();
		for (int key_index = 0; key_index < IM_ARRAYSIZE(io.KeysDown); key_index++) {
			if (ImGui::IsKeyDown(key_index)) {
				machine_runner->set_key_down(key_index);
			}
			if (ImGui::IsKeyReleased(key_index)) {
				machine_runner->set_key_up(key_index);
			}
		}
		ImGui::End();

		if (ImGui::Begin("Palette")) {
			byte* palette_image_data = palette_texture.data();
			for (int c = 0; c != 256; ++c) {
				byte r = dac_ram[3 * c + 0];
				byte g = dac_ram[3 * c + 1];
				byte b = dac_ram[3 * c + 2];

				palette_image_data[4 * c + 0] = (r << 2) | (r >> 4);
				palette_image_data[4 * c + 1] = (g << 2) | (g >> 4);
				palette_image_data[4 * c + 2] = (b << 2) | (b >> 4);
				palette_image_data[4 * c + 3] = 255;
			}
			palette_texture.apply();

			ImGui::Image((ImTextureID)palette_texture.id(), ImVec2(8 * palette_texture.width(), 8 * palette_texture.height()));

			ImGui::End();
		}

		if (ImGui::Begin("Framebuffer Mouse State")) {
			ImGui::Text("X: %d", frame_x);
			ImGui::Text("Y: %d", frame_y);
			ImGui::Text("Button: %d", mouse_btn);
			ImGui::End();
		}

		// Display Keyboard/Mouse state
		if (ImGui::Begin("Keyboard & Navigation State")) {
			ImGuiIO& io = ImGui::GetIO();
			ImGui::Text("Keys down:");          for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) if (ImGui::IsKeyDown(i)) { ImGui::SameLine(); ImGui::Text("%d (0x%X) (%.02f secs)", i, i, io.KeysDownDuration[i]); }
			ImGui::Text("Keys pressed:");       for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) if (ImGui::IsKeyPressed(i)) { ImGui::SameLine(); ImGui::Text("%d (0x%X)", i, i); }
			ImGui::Text("Keys release:");       for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) if (ImGui::IsKeyReleased(i)) { ImGui::SameLine(); ImGui::Text("%d (0x%X)", i, i); }
			ImGui::Text("Keys mods: %s%s%s%s", io.KeyCtrl ? "CTRL " : "", io.KeyShift ? "SHIFT " : "", io.KeyAlt ? "ALT " : "", io.KeySuper ? "SUPER " : "");
			ImGui::Text("Chars queue:");        for (int i = 0; i < io.InputQueueCharacters.Size; i++) { ImWchar c = io.InputQueueCharacters[i]; ImGui::SameLine();  ImGui::Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c); }

			ImGui::Text("NavInputs down:");     for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++) if (io.NavInputs[i] > 0.0f) { ImGui::SameLine(); ImGui::Text("[%d] %.2f (%.02f secs)", i, io.NavInputs[i], io.NavInputsDownDuration[i]); }
			ImGui::Text("NavInputs pressed:");  for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++) if (io.NavInputsDownDuration[i] == 0.0f) { ImGui::SameLine(); ImGui::Text("[%d]", i); }
			ImGui::End();
		}

		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	machine_runner->stop();
}

void main_window_t::uninitialize_imgui() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void main_window_t::uninitialize_glfw() {
	glfwDestroyWindow(window);
	glfwWaitEvents();
	glfwTerminate();
}
