
#version 460
#extension GL_ARB_bindless_texture : require

#pragma PROGRAM_COMPUTE
#include includes/VolumeFogShared.glsl

layout(local_size_x = 32, local_size_y = 2, local_size_z = 1) in;
void main()
{
	float4 accumulation = float4(0, 0, 0, 1);
	int3 pos = int3(gl_GlobalInvocationID.xy, 0);

	for (uint z = 0; z < VOLUME_DEPTH; ++z)
	{
		pos.z = int(z);

		float slicewidth = GetVolumeSliceWidth(pos.z);

		float4 slice = imageLoad(pk_Volume_Inject, pos);
		float density = max(slice.a, 0.000001f);
		float3 light = slice.rgb;

		float  transmittance = exp(-density * slicewidth);
		float3 lightintegral = light * (1.0f - transmittance) / density;

		accumulation.rgb += lightintegral * accumulation.a;
		accumulation.a *= transmittance;

		float4 preval = imageLoad(pk_Volume_Scatter, pos);
		float4 outval = lerp(preval, accumulation, VOLUME_ACCUMULATION_SC);

		imageStore(pk_Volume_Scatter, pos, outval);
	}
}