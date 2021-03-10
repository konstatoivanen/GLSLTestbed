#pragma once
#include "PrecompiledHeader.h"
#include "Input.h"

struct ApplicationConfig
{
	bool enable_console;
	bool enable_vsync;
	bool enable_verbose;
	bool enable_profiler;
	bool enable_orthographic;
	
	int	window_width;
	int	window_height;
	
	float camera_move_speed;
	float camera_field_o_fview;
	float camera_orthographic_size;
	float camera_clip_near;
	float camera_clip_far;
	
	float time_scale;

	KeyCode input_shader_next;
	KeyCode input_shader_reimport;
	KeyCode input_shader_list_uniforms;
	KeyCode input_timescale_increase;
	KeyCode input_timescale_decrease;
	KeyCode input_exit;

	KeyCode input_move_forward;
	KeyCode input_move_backward;
	KeyCode input_move_left;
	KeyCode input_move_right;
	KeyCode input_move_up;
	KeyCode input_move_down;

	ApplicationConfig(const std::string& filepath);
};

