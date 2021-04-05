#pragma once
#include "PrecompiledHeader.h"
#include "Input.h"

namespace PK::Core
{
	struct ApplicationConfig
	{
		bool EnableConsole;
		bool EnableVsync;
		bool EnableGizmos;
		bool EnableLightingDebug;
	
		int	WindowWidth;
		int	WindowHeight;
		
		uint RandomSeed;

		bool EnableOrthoGraphic;
		float CameraSpeed;
		float CameraFov;
		float CameraOrthoSize;
		float CameraZNear;
		float CameraZFar;
		
		float TimeScale;

		uint LightCount;
	
		float TonemapExposure;
		float BloomIntensity;
		float BloomLensDirtIntensity;
		std::string FileBloomDirt;

		float AmbientOcclusionIntensity;
		float AmbientOcclusionRadius;
		bool AmbientOcclusionDownsample;

		std::string FileBackgroundTexture;
		float BackgroundExposure;
	
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
}