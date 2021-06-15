#version 460

#include includes/PKCommon.glsl
#include includes/Reconstruction.glsl
#include includes/Encoding.glsl
#include includes/SceneGIShared.glsl

#pragma PROGRAM_VERTEX
layout(location = 0) in float4 in_POSITION0;
layout(location = 1) in float2 in_TEXCOORD0;
out float2 vs_TEXCOORD0;

void main()
{
	gl_Position = in_POSITION0;
	vs_TEXCOORD0 = in_TEXCOORD0;
};

#pragma PROGRAM_FRAGMENT

#define SAMPLE_COUNT 16

const float3 SAMPLES_HAMMERSLEY_3D[SAMPLE_COUNT] =
{
	float3(1.0f,0.0f,0.0f),
	float3(0.9238795f,0.3826835f,0.5f),
	float3(0.7071068f,0.7071068f,0.25f),
	float3(0.3826834f,0.9238795f,0.75f),
	float3(-4.371139E-08f,1.0f,0.125f),
	float3(-0.3826835f,0.9238795f,0.625f),
	float3(-0.7071068f,0.7071068f,0.375f),
	float3(-0.9238796f,0.3826833f,0.875f),
	float3(-1.0f,-8.742278E-08f,0.0625f),
	float3(-0.9238795f,-0.3826834f,0.5625f),
	float3(-0.7071066f,-0.7071069f,0.3125f),
	float3(-0.3826831f,-0.9238797f,0.8125f),
	float3(1.192488E-08f,-1.0f,0.1875f),
	float3(0.3826836f,-0.9238794f,0.6875f),
	float3(0.707107f,-0.7071065f,0.4375f),
	float3(0.9238796f,-0.3826834f,0.9375f),
};

float Attenuate(float dist) { return 1.0f / (1e-4f + dist * dist); }

float3 DistributeHammersley3D(float3 Xi, float blur)
{
	float theta = (1.0f - Xi.z) / (1.0f + (blur - 1.0f) * Xi.z);
	float2 sincos = sqrt(float2(1.0f - theta, theta));
	return normalize(float3(Xi.xy * sincos.xx, sincos.y));
}

// Source: https://research.nvidia.com/sites/default/files/publications/GIVoxels-pg2011-authors.pdf
float4 ConeTrace(const float3 origin, float3 direction) 
{
	const float CONE_SPREAD = 1.0;

	float4 color = 0.0f.xxxx;
	float dist = VOXEL_SIZE * 2;
	uint iteration = 0;

	while (iteration++ < 32 && color.a < 1.0f)
	{
		float level = log2(CONE_SPREAD * dist / VOXEL_SIZE);
		float ll = pow2(level + 1);
		float4 voxel = SampleSceneGI(origin + dist * direction, level);
		voxel.rgb *= Attenuate(dist);
		color += voxel;
		dist += ll * VOXEL_SIZE * 2;
	}

	return color;
}

in float2 vs_TEXCOORD0;
layout(location = 0) out float4 SV_Target0;
void main() 
{
	float3 worldposition = SampleWorldPosition(vs_TEXCOORD0);

	if (Greater(abs(WorldToClipSpace(worldposition)), 1.0f.xxx))
	{
		SV_Target0 = float4(0.0f.xxx, 1.0f);
		return;
	}

	// Find a base for the side cones with the normal as one of its base vectors.
	const float3 N = SampleWorldSpaceNormal(vs_TEXCOORD0);
	const float3 U = abs(N.z) < 0.999f ? half3(0.0f, 0.0f, 1.0f) : half3(1.0f, 0.0f, 0.0f);
	const float3 T = normalize(cross(U, N));
	const float3 B = cross(N, T);
	const float3 O = worldposition + N * (1 + PK_INV_SQRT2) * VOXEL_SIZE;

	float2 noiseuv = vs_TEXCOORD0 * (pk_ScreenParams.xy / 256.0f);

	float4 A = 0.0f.xxxx;

	#pragma unroll SAMPLE_COUNT
	for (uint i = 0u; i < SAMPLE_COUNT; ++i)
	{
		float3 H = DistributeHammersley3D(SAMPLES_HAMMERSLEY_3D[i], 1.0f);
		float3 dither = GlobalNoiseBlueUV(noiseuv + H.xy);
		H += (dither - 0.5f) * 0.5f;
		H = normalize(T * H.x + B * H.y + N * H.z);

		//faceforward(H, -N, H);

		float4 V = ConeTrace(O, H);
		V.rgb *= max(dot(N, H), 0.0f);

		A += V;
	}

	A /= SAMPLE_COUNT;
	A.a = saturate(1.0f - A.a);

	SV_Target0 = A;
}