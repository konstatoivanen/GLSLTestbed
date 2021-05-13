#version 460
#extension GL_ARB_bindless_texture : require

#pragma PROGRAM_COMPUTE
#include includes/VolumeFogShared.glsl

layout(local_size_x = 32, local_size_y = 2, local_size_z = 1) in;
void main()
{
	float4 accumulation = float4(0, 0, 0, 1);
	int3 pos = int3(gl_GlobalInvocationID.xy, 0);

	#pragma unroll VOLUME_DEPTH
	for (;pos.z < VOLUME_DEPTH; ++pos.z)
	{
		float slicewidth = GetVolumeSliceWidth(pos.z);

		float4 slice = imageLoad(pk_Volume_Inject, pos);

		float  transmittance = exp(-slice.a * slicewidth);
		float3 lightintegral = slice.rgb * (1.0f - transmittance) / slice.a;

		accumulation.rgb += lightintegral * accumulation.a;
		accumulation.a *= transmittance;

		float4 preval = imageLoad(pk_Volume_Scatter, pos);
		float4 outval = lerp(preval, accumulation, VOLUME_ACCUMULATION_SC);

		imageStore(pk_Volume_Scatter, pos, outval);
	}
}