#include "gui/main_window.h"

#include "emu/i8086.h"
#include "emu/ibm5160.h"
#include "emu/vga.h"
#include "disasm/disasm_i8086.h"
#include "gui/disassembler_view.h"
#include "gui/machine_runner.h"
#include "gui/texture.h"

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui.h>
#include <imgui_demo.cpp>

#include <cstdio>
#include <thread>
#include <imgui_memory_editor.h>

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

	ImGuiIO &io = ImGui::GetIO();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 150");
}

void main_window_t::loop() {
	texture_t frame_texture(320, 200);
	byte dac_ram[0x300];

	// bool show_demo_window = true;

		// bool show_demo_window = true;
	bool show_disassembler = true;

	auto disassembler_view = new disassembler_view_t;

	disassembler_view->focus({ 0x01ed, 0x0000 });

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuiIO& io = ImGui::GetIO();
		if (io.KeyAlt && io.KeysDown[GLFW_KEY_PAUSE]) {
			machine_runner->pause();
		}
		if (io.KeysDown[GLFW_KEY_F5]) {
			machine_runner->resume();
		}
		if (machine_runner->is_paused() && io.KeysDown[GLFW_KEY_F10]) {
			machine_runner->debug_run(1);
		}

		machine_runner->with_machine([&](ibm5160_t *machine) {
			disassembler_view->draw("Disassembler", &show_disassembler, [&machine](address_space_t s, uint32_t addr, width_t w) { return machine->read(s, addr, w); });
			machine->vga->read_rgba(frame_texture.data(), 0xA0000, frame_texture.width(), frame_texture.height());
			machine->vga->read_dac_ram(dac_ram);
			});

		int frame_x = 0;
		int frame_y = 0;
		uint16_t mouse_btn = 0;

		create_window_framebuffer(frame_texture, frame_x, frame_y, mouse_btn);
		capture_keyboard();
		create_window_palette_state(dac_ram);
		create_window_mouse_state(frame_x, frame_y, mouse_btn);
		create_window_cpu_state();
		create_window_disasm();
		create_window_hexview();

		glfw_render_frame();
	}

	machine_runner->stop();
}

void main_window_t::create_window_hexview() {
	static MemoryEditor mem_edit_2;
	if (ImGui::Begin("Memory View"))
	{
		machine_runner->with_machine([&](ibm5160_t* machine) {
			mem_edit_2.DrawContents(machine->memory, MEMORY_SIZE);
			});
		ImGui::End();
	}
}

void main_window_t::create_window_disasm() {
	if (ImGui::Begin("Disassembly")) {
		machine_runner->with_machine([&](ibm5160_t* machine) {
			if (ImGui::Button("Pause")) {
				machine_runner->pause();
			}
			ImGui::SameLine();
			if (ImGui::Button("Resume")) {
				machine_runner->resume();
			}
			if (machine_runner->is_paused()) {
				ImGui::SameLine();
				if (ImGui::Button("Step over")) {
					machine_runner->debug_run(1);
				}
			}
			disasm_i8086_t disasm;
			disasm.read = [&machine](address_space_t s, uint32_t addr, width_t w) { return machine->read(s, addr, w); };
			disasm.always_show_mem_width = true;
			uint16_t cs = machine->cpu->cs;
			uint16_t ip = machine->cpu->ip;
			for (int i = 0; i != 20; ++i) {
				if (i == 0) {
					ImGui::Text("-> ");
					ImGui::SameLine();
				}
				const char* str;
				disasm.disassemble(cs, &ip, &str);
				ImGui::Text(str);
			}
			});
		ImGui::End();
	}
}
void main_window_t::glfw_render_frame() {
	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(window);
}

void main_window_t::create_window_framebuffer(texture_t& frame_texture, int& frame_x, int& frame_y, uint16_t& mouse_btn) {
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
		ImGui::End();
	}
}

void main_window_t::capture_keyboard() {
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
}

void main_window_t::create_window_palette_state(byte dac_ram[768]) {
	if (ImGui::Begin("Palette")) {
		texture_t palette_texture(16, 16);
		byte *palette_image_data = palette_texture.data();
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
}

void main_window_t::create_window_mouse_state(int frame_x, int frame_y, const uint16_t& mouse_btn) {
	if (ImGui::Begin("Framebuffer Mouse State")) {
		ImGui::Text("X: %d", frame_x);
		ImGui::Text("Y: %d", frame_y);
		ImGui::Text("Button: %d", mouse_btn);
		ImGui::End();
	}
}

void main_window_t::create_window_cpu_state() {
	machine_runner->with_machine([&](ibm5160_t *machine) {
		if (ImGui::Begin("CPU State")) {
			if (ImGui::BeginTable("Registers", 2,
				ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInner
				| ImGuiTableBgTarget_CellBg | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Reorderable))
				{
					i8086_t *cpu = (i8086_t *)machine->cpu;

					ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
					ImGui::TableNextColumn();
					ImGui::Text("Field");
					ImGui::TableNextColumn();
					ImGui::Text("Value");
					ImGui::TableNextColumn();
					ImGui::Text("AX");
					ImGui::TableNextColumn();
					ImGui::Text("%04x", cpu->ax);
					ImGui::TableNextColumn();
					ImGui::Text("ES");
					ImGui::TableNextColumn();
					ImGui::Text("%04x", cpu->es);
					ImGui::TableNextColumn();
					ImGui::Text("SP");
					ImGui::TableNextColumn();
					ImGui::Text("%04x", cpu->sp);
					ImGui::TableNextColumn();
					ImGui::Text("IP");
					ImGui::TableNextColumn();
					ImGui::Text("%04x", cpu->ip);
					ImGui::TableNextColumn();
					ImGui::Text("CX");
					ImGui::TableNextColumn();
					ImGui::Text("%04x", cpu->cx);
					ImGui::TableNextColumn();
					ImGui::Text("CS");
					ImGui::TableNextColumn();
					ImGui::Text("%04x", cpu->cs);
					ImGui::TableNextColumn();
					ImGui::Text("BP");
					ImGui::TableNextColumn();
					ImGui::Text("%04x", cpu->bp);
					ImGui::TableNextColumn();
					ImGui::Text("OP");
					ImGui::TableNextColumn();
					ImGui::Text("%04x", cpu->op);
					ImGui::TableNextColumn();
					ImGui::Text("DX");
					ImGui::TableNextColumn();
					ImGui::Text("%04x", cpu->dx);
					ImGui::TableNextColumn();
					ImGui::Text("SS");
					ImGui::TableNextColumn();
					ImGui::Text("%04x", cpu->ss);
					ImGui::TableNextColumn();
					ImGui::Text("DI");
					ImGui::TableNextColumn();
					ImGui::Text("%04x", cpu->di);
					ImGui::TableNextColumn();
					ImGui::Text("BX");
					ImGui::TableNextColumn();
					ImGui::Text("%04x", cpu->bx);
					ImGui::TableNextColumn();
					ImGui::Text("DS");
					ImGui::TableNextColumn();
					ImGui::Text("%04x", cpu->ds);
					ImGui::TableNextColumn();
					ImGui::Text("SI");
					ImGui::TableNextColumn();
					ImGui::Text("%04x", cpu->si);
					ImGui::TableNextColumn();
					ImGui::Text("OF");
					ImGui::TableNextColumn();
					ImGui::Text("%d", cpu->get_of());
					ImGui::TableNextColumn();
					ImGui::Text("DF");
					ImGui::TableNextColumn();
					ImGui::Text("%d", cpu->get_df());
					ImGui::TableNextColumn();
					ImGui::Text("IF");
					ImGui::TableNextColumn();
					ImGui::Text("%d", cpu->get_if());
					ImGui::TableNextColumn();
					ImGui::Text("TF");
					ImGui::TableNextColumn();
					ImGui::Text("%d", cpu->get_tf());
					ImGui::TableNextColumn();
					ImGui::Text("SF");
					ImGui::TableNextColumn();
					ImGui::Text("%d", cpu->get_sf());
					ImGui::TableNextColumn();
					ImGui::Text("ZF");
					ImGui::TableNextColumn();
					ImGui::Text("%d", cpu->get_zf());
					ImGui::TableNextColumn();
					ImGui::Text("AF");
					ImGui::TableNextColumn();
					ImGui::Text("%d", cpu->get_af());
					ImGui::TableNextColumn();
					ImGui::Text("PF");
					ImGui::TableNextColumn();
					ImGui::Text("%d", cpu->get_pf());
					ImGui::TableNextColumn();
					ImGui::Text("CF");
					ImGui::TableNextColumn();
					ImGui::Text("%d", cpu->get_cf());
					ImGui::TableNextColumn();
					ImGui::Text("Combined Flags");
					ImGui::TableNextColumn();
					ImGui::Text("%04X", cpu->get_log_flags());
					ImGui::TableNextColumn();
					ImGui::Text("Instruction count");
					ImGui::TableNextColumn();
					ImGui::Text("%d", cpu->get_instr_count());
					ImGui::TableNextColumn();
					ImGui::Text("OP");
					ImGui::TableNextColumn();
					ImGui::Text("%02X", cpu->op);
					ImGui::TableNextColumn();
					ImGui::Text("Is Prefix");
					ImGui::TableNextColumn();
					ImGui::Text("%d", cpu->is_prefix);
					ImGui::TableNextColumn();
					ImGui::Text("Int delay");
					ImGui::TableNextColumn();
					ImGui::Text("%d", cpu->int_delay);
					ImGui::TableNextColumn();
					ImGui::Text("Int nmi");
					ImGui::TableNextColumn();
					ImGui::Text("%d", cpu->int_nmi);
					ImGui::TableNextColumn();
					ImGui::Text("Int intr");
					ImGui::TableNextColumn();
					ImGui::Text("%d", cpu->int_intr);
					ImGui::TableNextColumn();
					ImGui::Text("Int Number");
					ImGui::TableNextColumn();
					ImGui::Text("%d", cpu->int_number);
					ImGui::EndTable();
				}
				ImGui::End();
			}
		});
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
