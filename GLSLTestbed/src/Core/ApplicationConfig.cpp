#include "PrecompiledHeader.h"
#include "Utilities/Log.h"
#include "Utilities/StringUtilities.h"
#include "Core/ApplicationConfig.h"
#include <GLFW\glfw3.h>
#include <yaml-cpp/yaml.h>

namespace PK::Core
{
	template<typename T>
	T TryParse(const YAML::Node& node, const T& _default)
	{
		return node ? node.as<T>() : _default;
	}
	
	ApplicationConfig::ApplicationConfig(const std::string& filepath)
	{
		YAML::Node root = YAML::LoadFile(filepath);
	
		PK_CORE_ASSERT(root, "Failed to open config file at: %s", filepath.c_str());
	
		auto properties = root["Properties"];
	
		PK_CORE_ASSERT(properties, "Config (%s) doesn't contain any properties!", filepath.c_str());
	
		EnableConsole = TryParse(properties["EnableConsole"], false);
		EnableVsync = TryParse(properties["EnableVsync"], false);
		EnableGizmos = TryParse(properties["EnableGizmos"], false);
		EnableLightingDebug = TryParse(properties["EnableLightingDebug"], false);
		EnableOrthoGraphic = TryParse(properties["EnableOrthoGraphic"], false);
		WindowWidth = TryParse(properties["WindowWidth"], 512);
		WindowHeight = TryParse(properties["WindowHeight"], 512);
		CameraSpeed = TryParse(properties["CameraSpeed"], 5.0f);
		CameraLookSensitivity = TryParse(properties["CameraLookSensitivity"], 1.0f);
		CameraMoveSmoothing = TryParse(properties["CameraMoveSmoothing"], 0.0f);
		CameraLookSmoothing = TryParse(properties["CameraLookSmoothing"], 0.0f);
		CameraFov = TryParse(properties["CameraFov"], 75.0f);
		CameraOrthoSize = TryParse(properties["CameraOrthoSize"], 10.0f);
		CameraZNear = TryParse(properties["CameraZNear"], 0.1f);
		CameraZFar = TryParse(properties["CameraZFar"], 200.0f);
		CascadeLinearity = TryParse(properties["CascadeLinearity"], 0.5f);
		TimeScale = TryParse(properties["TimeScale"], 1.0f); 

		RandomSeed = TryParse(properties["RandomSeed"], 0u);

		LightCount = TryParse(properties["LightCount"], 256);
	
		TonemapExposure = TryParse(properties["TonemapExposure"], 1.0f);
		BloomIntensity = TryParse(properties["BloomIntensity"], 0.0f);
		BloomLensDirtIntensity = TryParse(properties["BloomLensDirtIntensity"], 0.0f);
		FileBloomDirt = TryParse(properties["FileBloomDirt"], std::string("T_Bloom_LensDirt"));

		AmbientOcclusionIntensity = TryParse(properties["AmbientOcclusionIntensity"], 1.0f);
		AmbientOcclusionRadius = TryParse(properties["AmbientOcclusionRadius"], 1.0f);
		AmbientOcclusionDownsample = TryParse(properties["AmbientOcclusionDownsample"], true);

		VolumeConstantFog = TryParse(properties["VolumeConstantFog"], 0.0f);
		VolumeHeightFogExponent = TryParse(properties["VolumeHeightFogExponent"], 0.0f);
		VolumeHeightFogOffset = TryParse(properties["VolumeHeightFogOffset"], 0.0f);
		VolumeHeightFogAmount = TryParse(properties["VolumeHeightFogAmount"], 0.0f);
		VolumeDensity = TryParse(properties["VolumeDensity"], 0.0f);
		VolumeIntensity = TryParse(properties["VolumeIntensity"], 0.0f);
		VolumeAnisotropy = TryParse(properties["VolumeAnisotropy"], 0.0f);
		VolumeNoiseFogAmount = TryParse(properties["VolumeNoiseFogAmount"], 0.0f);
		VolumeNoiseFogScale = TryParse(properties["VolumeNoiseFogScale"], 0.0f);
		VolumeWindSpeed = TryParse(properties["VolumeWindSpeed"], 0.0f);

		FileBackgroundTexture = TryParse(properties["FileBackgroundTexture"], std::string("T_OEM_Mountains"));
		BackgroundExposure = TryParse(properties["BackgroundExposure"], 1.0f);
	
		auto prop = properties["ButtonShaderNext"];
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
}