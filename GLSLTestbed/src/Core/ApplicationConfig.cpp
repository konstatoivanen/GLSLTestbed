#include "PrecompiledHeader.h"
#include "Utilities/Log.h"
#include "Utilities/StringUtilities.h"
#include "Core/ApplicationConfig.h"
#include "Core/YamlSerializers.h"
#include <GLFW\glfw3.h>

namespace PK::Core
{
	ApplicationConfig::ApplicationConfig()
	{
		values =
		{
			&EnableConsole,
			&EnableVsync,
			&EnableGizmos,
			&EnableLightingDebug,
			&EnableCursor,
			&InitialWidth,
			&InitialHeight,
			&CameraSpeed,
			&CameraLookSensitivity,
			&CameraMoveSmoothing,
			&CameraLookSmoothing,
			&CameraFov,
			&CameraZNear,
			&CameraZFar,
			&CascadeLinearity,
			&TimeScale,
			&RandomSeed,
			&LightCount,
			&ShadowmapTileSize,
			&ShadowmapTileCount,
			&CameraFocalLength,
			&CameraFNumber,
			&CameraFilmHeight,
			&CameraFocusSpeed,
			&AutoExposureLuminanceMin,
			&AutoExposureLuminanceRange,
			&AutoExposureSpeed,
			&TonemapExposure,
			&VignetteIntensity,
			&VignettePower,
			&FilmGrainIntensity,
			&FilmGrainLuminance,
			&CC_Contribution,
			&CC_TemperatureShift,
			&CC_Tint,
			&CC_Hue,
			&CC_Saturation,
			&CC_Vibrance,
			&CC_Value,
			&CC_Contrast,
			&CC_Gain,
			&CC_Gamma,
			&CC_Shadows,
			&CC_Midtones,
			&CC_Highlights,
			&CC_ChannelMixerRed,
			&CC_ChannelMixerGreen,
			&CC_ChannelMixerBlue,
			&BloomIntensity,
			&BloomLensDirtIntensity,
			&FileBloomDirt,
			&AmbientOcclusionIntensity,
			&AmbientOcclusionRadius,
			&AmbientOcclusionDownsample,
			&DownscaleGI,
			&VolumeConstantFog,
			&VolumeHeightFogExponent,
			&VolumeHeightFogOffset,
			&VolumeHeightFogAmount,
			&VolumeDensity,
			&VolumeIntensity,
			&VolumeAnisotropy,
			&VolumeNoiseFogAmount,
			&VolumeNoiseFogScale,
			&VolumeWindSpeed,
			&FileBackgroundTexture,
			&BackgroundExposure,
		};
	}

	template<>
	bool AssetImporters::IsValidExtension<ApplicationConfig>(const std::filesystem::path& extension) { return extension.compare(".cfg") == 0; }

	template<>
	void AssetImporters::Import(const std::string& filepath, Ref<ApplicationConfig>& config) { config->Load(filepath); }
}