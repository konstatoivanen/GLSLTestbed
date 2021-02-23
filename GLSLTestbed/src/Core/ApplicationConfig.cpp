#include "PrecompiledHeader.h"
#include "Utilities/Log.h"
#include "Utilities/StringUtilities.h"
#include "Core/ApplicationConfig.h"
#include <GLFW\glfw3.h>

ApplicationConfig::ApplicationConfig(const std::string& filepath)
{
	std::ifstream file(filepath);

	PK_CORE_ASSERT(!file.fail(), "Failed to open config file at: %s", filepath.c_str());

	std::string line;
	std::unordered_map<std::string, std::string> variables;

	while (std::getline(file, line))
	{
		if (line.find("#") == line.npos)
		{
			continue;
		}

		line.erase(0, 1);

		auto spaceIdx = line.find(" ");

		if (spaceIdx == line.npos || spaceIdx == line.length() - 1)
		{
			continue;
		}

		auto name = line.substr(0, spaceIdx);
		auto value = StringUtilities::Trim(line.erase(0, spaceIdx));
		variables[name] = value;
	}

	enable_console = variables.count("enable_console") ? stoi(variables["enable_console"]) > 0 : false;
	enable_vsync = variables.count("enable_vsync") ? stoi(variables["enable_vsync"]) > 0 : false;
	enable_verbose = variables.count("enable_verbose") ? stoi(variables["enable_verbose"]) > 0 : false;
	enable_profiler = variables.count("enable_profiler") ? stoi(variables["enable_profiler"]) > 0 : false;
	enable_orthographic = variables.count("enable_orthographic") ? stoi(variables["enable_orthographic"]) > 0 : false;

	window_width = variables.count("window_width") ? stoi(variables["window_width"]) : 512;
	window_height = variables.count("window_height") ? stoi(variables["window_height"]) : 512;

	camera_move_speed = variables.count("camera_move_speed") ? stof(variables["camera_move_speed"]) : 1.0f;
	camera_field_o_fview = variables.count("camera_field_o_fview") ? stof(variables["camera_field_o_fview"]) : 90.0f;
	camera_orthographic_size = variables.count("camera_orthographic_size") ? stof(variables["camera_orthographic_size"]) : 10.0f;
	camera_clip_near = variables.count("camera_clip_near") ? stof(variables["camera_clip_near"]) : 0.1f;
	camera_clip_far = variables.count("camera_clip_far") ? stof(variables["camera_clip_far"]) : 100.0f;

	time_scale = variables.count("time_scale") ? stof(variables["time_scale"]) : 1.0f;

	input_shader_next = variables.count("input_shader_next") ? stoi(variables["input_shader_next"]) : GLFW_KEY_Z;
	input_shader_reimport = variables.count("input_shader_reimport") ? stoi(variables["input_shader_reimport"]) : GLFW_KEY_X;
	input_shader_list_uniforms = variables.count("input_shader_list_uniforms") ? stoi(variables["input_shader_list_uniforms"]) : GLFW_KEY_B;
	input_timescale_increase = variables.count("input_timescale_increase") ? stoi(variables["input_timescale_increase"]) : GLFW_KEY_C;
	input_timescale_decrease = variables.count("input_timescale_decrease") ? stoi(variables["input_timescale_decrease"]) : GLFW_KEY_V;
	input_exit = variables.count("input_exit") ? stoi(variables["input_exit"]) : GLFW_KEY_ESCAPE;

	input_move_forward = variables.count("input_move_forward") ? stoi(variables["input_move_forward"]) : GLFW_KEY_W;
	input_move_backward = variables.count("input_move_backward") ? stoi(variables["input_move_backward"]) : GLFW_KEY_S;
	input_move_left = variables.count("input_move_left") ? stoi(variables["input_move_left"]) : GLFW_KEY_A;
	input_move_right = variables.count("input_move_right") ? stoi(variables["input_move_right"]) : GLFW_KEY_D;
	input_move_up = variables.count("input_move_up") ? stoi(variables["input_move_up"]) : GLFW_KEY_Q;
	input_move_down = variables.count("input_move_down") ? stoi(variables["input_move_down"]) : GLFW_KEY_E;
}