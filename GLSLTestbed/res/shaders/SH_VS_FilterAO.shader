#version 460

#multi_compile AO_PASS0 AO_PASS1 AO_PASS2

#include includes/PKCommon.glsl
#include includes/Reconstruction.glsl
#include includes/Noise.glsl

const float kNDotVBias = 0.002;
const float kGeometryAwareness = 50;
const int SAMPLE_COUNT = 16;

uniform sampler2D _MainTex;
uniform half _Intensity;
uniform float _Radius;
uniform float _TargetScale;
uniform float2 _BlurVector;

#define _Maintex_TexelSize (float2(1.0f, 1.0f) / textureSize(_MainTex, 0).xy)

float ViewPortClip(float2 uv, float d)
{
    float ob = (Less(uv, float2(0.0f)) ? 1.0f : 0.0f) + (Greater(uv, float2(1.0f)) ? 1.0f : 0.0f);
    ob += (d >= 0.99999) ? 1.0f : 0.0f;
    return ob * 1e8;
}

float SampleDepth(float2 uv)
{
    float d = tex2D(pk_ScreenDepth, uv).r;
    return LinearizeDepth(d) + ViewPortClip(uv, d);
}                      

float GetDepthBlurFactor(float2 uv)
{
    //@TODO solve this correctly later
    float d = 1.0f - (LinearizeDepth(tex2D(pk_ScreenDepth, uv).r) - pk_ProjectionParams.y) / pk_ProjectionParams.z;
    return d * d * d * d * d * d * d * d * d * d * d * d * d * d * d * d;
}

float CompareNormal(float3 normal0, float3 normal1)
{
    return pow((dot(normal0, normal1) + 1) * 0.5f, kGeometryAwareness);
}

float3 GetSampleDirection(float2 uv, float3 normal, float index)
{
    float ngrad = NoiseGradient(uv * _TargetScale, pk_ScreenParams.xy);

    float theta = (NoiseUV(1, index) + ngrad) * PK_PI * 2;

    float nuv = fract(NoiseUV(0, index) + ngrad) * 2 - 1;

    float3 direction = float3(float2(cos(theta), sin(theta)) * sqrt(1 - nuv * nuv), nuv);

    direction *= sqrt((index + 1) / SAMPLE_COUNT) * _Radius;

    return faceforward(direction, -normal, direction);
}

float ComputeAO(float3 P, float3 N, float3 S, float D)
{
    float3 V = S - P;
    float VdotV = dot(V, V) + 1e-4;
    float NdotV = max(0, dot(N, V) - kNDotVBias * D);
    return NdotV / VdotV;
}

float ComputeCoarseAO(float2 uv)
{
    float3x3 projection = float3x3(pk_MATRIX_P);

    float vdepth = SampleDepth(uv);
    float3 vnormal = SampleViewSpaceNormal(uv);
    float3 vposition = ClipToViewPos(uv, vdepth);

    float AO = 0.0f;

    for (int index = 0; index < SAMPLE_COUNT; ++index)
    {
        float3 direction = GetSampleDirection(uv, vnormal, index);
        float3 vposition1 = vposition + direction;

        float3 clippos = mul(projection, vposition1);
        float2 screenuv = (clippos.xy / vposition1.z + 1) * 0.5;

        float sampleDepth = SampleDepth(screenuv);
        float3 samplePosition = ClipToViewPos(screenuv, sampleDepth);

        AO += ComputeAO(vposition, vnormal, samplePosition, vdepth);
    }

    return pow(AO * _Radius * _Intensity / SAMPLE_COUNT, 0.6f);
}

half SeparableBlurLarge(float2 uv)
{
    float2 delta = _Maintex_TexelSize.xy * _BlurVector * GetDepthBlurFactor(uv);

    float2 uv10 = uv - delta * 1.0f;
    float2 uv11 = uv + delta * 1.0f;
    float2 uv20 = uv - delta * 2.0f;
    float2 uv21 = uv + delta * 2.0f;
    float2 uv30 = uv - delta * 3.2307692308f;
    float2 uv31 = uv + delta * 3.2307692308f;

    float3 normal = SampleViewSpaceNormal(uv);

    float w00 = 0.37004405286f;
    float w10 = CompareNormal(normal, SampleViewSpaceNormal(uv10)) * 0.31718061674f;
    float w11 = CompareNormal(normal, SampleViewSpaceNormal(uv11)) * 0.31718061674f;
    float w20 = CompareNormal(normal, SampleViewSpaceNormal(uv20)) * 0.19823788546f;
    float w21 = CompareNormal(normal, SampleViewSpaceNormal(uv21)) * 0.19823788546f;
    float w30 = CompareNormal(normal, SampleViewSpaceNormal(uv30)) * 0.11453744493f;
    float w31 = CompareNormal(normal, SampleViewSpaceNormal(uv31)) * 0.11453744493f;

    float s = tex2D(_MainTex, uv).r * w00;
    s += tex2D(_MainTex, uv10).r * w10;
    s += tex2D(_MainTex, uv11).r * w11;
    s += tex2D(_MainTex, uv20).r * w20;
    s += tex2D(_MainTex, uv21).r * w21;
    s += tex2D(_MainTex, uv30).r * w30;
    s += tex2D(_MainTex, uv31).r * w31;

    return s / (w00 + w10 + w11 + w20 + w21 + w30 + w31);
}

half SeparableBlurSmall(float2 uv)
{
    float2 delta = _Maintex_TexelSize.xy * _BlurVector * GetDepthBlurFactor(uv);

    float2 uv0 = uv - delta;
    float2 uv1 = uv + delta;

    float3 normal = SampleViewSpaceNormal(uv);

    float w1 = CompareNormal(normal, SampleViewSpaceNormal(uv0));
    float w2 = CompareNormal(normal, SampleViewSpaceNormal(uv1));

    float s = tex2D(_MainTex, uv).r * 2.0f;
    s += tex2D(_MainTex, uv0).r * w1;
    s += tex2D(_MainTex, uv1).r * w2;

    return s / (2.0f + w1 + w2);
}

#if defined(AO_PASS0)
#define FUNC_FRAG ComputeCoarseAO
#elif defined(AO_PASS1)
#define FUNC_FRAG SeparableBlurLarge
#elif defined(AO_PASS2)
#define FUNC_FRAG SeparableBlurSmall
#endif

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
in float2 vs_TEXCOORD0;
layout(location = 0) out float SV_Target0;
void main() { SV_Target0 = FUNC_FRAG(vs_TEXCOORD0); };