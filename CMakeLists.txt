cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(Chani)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Chani
file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.cpp)
add_executable(chani ${sources})
target_include_directories(chani PRIVATE src/)

# GLFW
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(3rdparty/glfw/)
target_include_directories(chani PRIVATE 3rdparty/glfw/include/)
target_sources(chani PRIVATE 3rdparty/imgui/examples/libs/gl3w/GL/gl3w.c)
target_link_libraries(chani glfw)

# ImGui
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GL3W)
target_include_directories(chani PRIVATE
	3rdparty/imgui/
	3rdparty/imgui/backends/
	3rdparty/imgui/examples/libs/gl3w/
)
target_sources(chani PRIVATE
	3rdparty/imgui/backends/imgui_impl_glfw.cpp
	3rdparty/imgui/backends/imgui_impl_opengl3.cpp
	3rdparty/imgui/imgui.cpp
	3rdparty/imgui/imgui_draw.cpp
	3rdparty/imgui/imgui_tables.cpp
	3rdparty/imgui/imgui_widgets.cpp
)

# Mac OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	target_link_libraries(chani stdc++ "-framework Foundation" "-framework Cocoa" objc)
endif()
