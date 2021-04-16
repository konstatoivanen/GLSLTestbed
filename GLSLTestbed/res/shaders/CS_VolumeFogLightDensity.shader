#version 460
#extension GL_ARB_bindless_texture : require

#pragma PROGRAM_COMPUTE
#include includes/Lighting.glsl
#include includes/VolumeResources.glsl
#include includes/Noise.glsl

float Density(float3 pos)
{
	float fog = pk_Volume_ConstantFog;

	fog += clamp(exp(pk_Volume_HeightFogExponent * (-pos.y + pk_Volume_HeightFogOffset)) * pk_Volume_HeightFogAmount, 0.0, 1e+2f);

	float3 warp = pos;

	fog *= NoiseScroll(warp, pk_Time.y * pk_Volume_WindSpeed, pk_Volume_NoiseFogScale, pk_Volume_WindDir.xyz, pk_Volume_NoiseFogAmount, -0.3, 8.0);

	return max(fog * pk_Volume_Density, 0.0);
}

layout(local_size_x = 16, local_size_y = 2, local_size_z = 16) in;
void main()
{
	uint3 id = gl_GlobalInvocationID;

	float2 uv = float2(id.x / VOLUME_WIDTH_FLOAT, id.y / VOLUME_HEIGHT_FLOAT);

	float depthslice = (pk_ProjectionParams.z - pk_ProjectionParams.y) / VOLUME_DEPTH;
	float lineardepth = lerp(pk_ProjectionParams.y, pk_ProjectionParams.z, float(id.z) / VOLUME_DEPTH);
	float3 worldpos = mul(pk_MATRIX_I_V, float4(ClipToViewPos(uv, lineardepth), 1.0f)).xyz;
	float3 viewdir = normalize(worldpos - pk_WorldSpaceCameraPos.xyz);
	float3 bluenoise = NoiseBlue(id.xy + id.z * int2(VOLUME_WIDTH, VOLUME_HEIGHT) + int(pk_Time.w * 1000).xx);
	float3 ambientdir = normalize(bluenoise - 0.5f);

	worldpos += viewdir * depthslice * NoiseUniformToTriangle(bluenoise.x);

	float3 color = SampleEnv(OctaUV(ambientdir), 1.0f);

	// @TODO Utilitize light tiles
	for (uint i = 0; i < pk_LightCount; ++i)
	{
		// color += GetVolumeLightColorAnistropic(i, worldpos, pk_Volume_Anisotropy);
		color += GetVolumeLightColor(i, worldpos);
	}
	
	float density = Density(worldpos);

	float4 preval = imageLoad(pk_Volume_Inject, int3(id));
	float4 curval = float4(pk_Volume_Intensity * density * color, density);
	
	imageStore(pk_Volume_Inject, int3(id),  lerp(preval, curval, 0.3f));
}