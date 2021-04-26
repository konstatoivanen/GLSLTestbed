#version 460
#extension GL_ARB_bindless_texture : require

#pragma PROGRAM_COMPUTE
#include includes/Lighting.glsl
#include includes/VolumeFogShared.glsl

float Density(float3 pos)
{
	float fog = pk_Volume_ConstantFog;

	fog += clamp(exp(pk_Volume_HeightFogExponent * (-pos.y + pk_Volume_HeightFogOffset)) * pk_Volume_HeightFogAmount, 0.0, 1e+3f);

	float3 warp = pos;

	fog *= NoiseScroll(warp, pk_Time.y * pk_Volume_WindSpeed, pk_Volume_NoiseFogScale, pk_Volume_WindDir.xyz, pk_Volume_NoiseFogAmount, -0.3, 8.0);

	return max(fog * pk_Volume_Density, 0.0);
}

float3 GetAmbientColor(float3 direction, float3 worldpos)
{
	float anistropy = GetLightAnistropy(worldpos, direction, pk_Volume_Anisotropy);
	return SampleEnv(OctaUV(direction), 1.0f) * anistropy;
}

layout(local_size_x = 16, local_size_y = 2, local_size_z = 16) in;
void main()
{
	uint3 id = gl_GlobalInvocationID;

	float3 bluenoise = GetVolumeCellNoise(id);
	float depth = GetVolumeCellDepth(id.z + NoiseUniformToTriangle(bluenoise.x));

	float2 uv = id.xy / VOLUME_SIZE_XY;

	float3 worldpos = mul(pk_MATRIX_I_V, float4(ClipToViewPos(uv, depth), 1.0f)).xyz;

	float3 color = GetAmbientColor(normalize(bluenoise - 0.5f + float3(0, 1, 0)), worldpos);

	LightTile tile = GetLightTile(GetTileIndex(uv * pk_ScreenParams.xy, depth));

	for (uint i = tile.start; i < tile.end; ++i)
	{
		color += GetVolumeLightColor(i, worldpos, pk_Volume_Anisotropy);
	}
	
	float density = Density(worldpos);

	float4 preval = imageLoad(pk_Volume_Inject, int3(id));
	float4 curval = float4(pk_Volume_Intensity * density * color, density);
	
	imageStore(pk_Volume_Inject, int3(id),  lerp(preval, curval, 0.2f));
}