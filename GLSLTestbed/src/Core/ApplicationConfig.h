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
		bool ShowCursor;

		int	InitialWidth;
		int	InitialHeight;
		
		uint RandomSeed;

		bool EnableOrthoGraphic;
		float CameraSpeed;
		float CameraLookSensitivity;
		float CameraMoveSmoothing;
		float CameraLookSmoothing;
		float CameraFov;
		float CameraOrthoSize;
		float CameraZNear;
		float CameraZFar;
		float CascadeLinearity;
		
		float TimeScale;

		uint LightCount;
	
		float CameraFocalLength;
		float CameraFNumber;
		float CameraFilmHeight;
		float CameraFocusSpeed;
		float AutoExposureLuminanceMin;
		float AutoExposureLuminanceRange;
		float AutoExposureSpeed;
		float TonemapExposure;
		float TonemapSaturation;
		float BloomIntensity;
		float BloomLensDirtIntensity;
		std::string FileBloomDirt;

		float AmbientOcclusionIntensity;
		float AmbientOcclusionRadius;
		bool AmbientOcclusionDownsample;

		float VolumeConstantFog;
		float VolumeHeightFogExponent;
		float VolumeHeightFogOffset;
		float VolumeHeightFogAmount;
		float VolumeDensity;
		float VolumeIntensity;
		float VolumeAnisotropy;
		float VolumeNoiseFogAmount;
		float VolumeNoiseFogScale;
		float VolumeWindSpeed;

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