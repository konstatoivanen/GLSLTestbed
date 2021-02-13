#pragma once
#include "PrecompiledHeader.h"

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

	int input_shader_next;
	int input_shader_reimport;
	int input_shader_list_uniforms;
	int input_timescale_increase;
	int input_timescale_decrease;
	int input_exit;

	int input_move_forward;
	int input_move_backward;
	int input_move_left;
	int input_move_right;
	int input_move_up;
	int input_move_down;

	ApplicationConfig(const std::string& filepath);
};

