#include "PrecompiledHeader.h"
#include "Utilities/Log.h"
#include "Utilities/StringUtilities.h"
#include "Core/ApplicationConfig.h"
#include <GLFW\glfw3.h>
#include <yaml-cpp/yaml.h>

ApplicationConfig::ApplicationConfig(const std::string& filepath)
{
	YAML::Node root = YAML::LoadFile(filepath);

	PK_CORE_ASSERT(root, "Failed to open config file at: %s", filepath.c_str());

	auto properties = root["Properties"];

	PK_CORE_ASSERT(properties, "Config (%s) doesn't contain any properties!", filepath.c_str());

	YAML::Node prop;

	prop = properties["EnableConsole"];
	enable_console = prop ? prop.as<bool>() : false;

	prop = properties["EnableVsync"];
	enable_vsync = prop ? prop.as<bool>() : false;

	prop = properties["EnableVerbose"];
	enable_verbose = prop ? prop.as<bool>() : false;

	prop = properties["EnableProfiler"];
	enable_profiler = prop ? prop.as<bool>() : false;

	prop = properties["EnableOrthoGraphic"];
	enable_orthographic = prop ? prop.as<bool>() : false;

	prop = properties["WindowWidth"];
	window_width = prop ? prop.as<int>() : 512;

	prop = properties["WindowHeight"];
	window_height = prop ? prop.as<int>() : 512;

	prop = properties["CameraSpeed"];
	camera_move_speed = prop ? prop.as<float>() : 5.0f;

	prop = properties["CameraFov"];
	camera_field_o_fview = prop ? prop.as<float>() : 75.0f;

	prop = properties["CameraOrthographicSize"];
	camera_orthographic_size = prop ? prop.as<float>() : 10.0f;

	prop = properties["CameraClipNear"];
	camera_clip_near = prop ? prop.as<float>() : 0.1f;

	prop = properties["CameraClipFar"];
	camera_clip_far = prop ? prop.as<float>() : 200.0f;

	prop = properties["TimeScale"];
	time_scale = prop ? prop.as<float>() : 1.0f;

	prop = properties["ButtonShaderNext"];
	input_shader_next = prop ? Input::StringToKey(prop.as<std::string>()) : KeyCode::X;

	prop = properties["ButtonShaderReimport"];
	input_shader_reimport = prop ? Input::StringToKey(prop.as<std::string>()) : KeyCode::R;

	prop = properties["ButtonListUniforms"];
	input_shader_list_uniforms = prop ? Input::StringToKey(prop.as<std::string>()) : KeyCode::L;

	prop = properties["ButtonTimescaleUp"];
	input_timescale_increase = prop ? Input::StringToKey(prop.as<std::string>()) : KeyCode::C;

	prop = properties["ButtonTimescaleDown"];
	input_timescale_decrease = prop ? Input::StringToKey(prop.as<std::string>()) : KeyCode::V;

	prop = properties["ButtonExit"];
	input_exit = prop ? Input::StringToKey(prop.as<std::string>()) : KeyCode::ESCAPE;

	prop = properties["ButtonMoveForward"];
	input_move_forward = prop ? Input::StringToKey(prop.as<std::string>()) : KeyCode::W;

	prop = properties["ButtonMoveBackward"];
	input_move_backward = prop ? Input::StringToKey(prop.as<std::string>()) : KeyCode::S;

	prop = properties["ButtonMoveLeft"];
	input_move_left = prop ? Input::StringToKey(prop.as<std::string>()) : KeyCode::A;

	prop = properties["ButtonMoveRight"];
	input_move_right = prop ? Input::StringToKey(prop.as<std::string>()) : KeyCode::D;

	prop = properties["ButtonMoveUP"];
	input_move_up = prop ? Input::StringToKey(prop.as<std::string>()) : KeyCode::Q;

	prop = properties["ButtonMoveDown"];
	input_move_down = prop ? Input::StringToKey(prop.as<std::string>()) : KeyCode::E;
}