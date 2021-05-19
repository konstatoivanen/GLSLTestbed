// Based on: https://github.com/keijiro/KinoObscurance
//
// Kino/Obscurance - Screen space ambient obscurance image effect
//
// Copyright (C) 2016 Keijiro Takahashi
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#version 460
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_viewport_layer_array : require

#multi_compile AO_PASS0 AO_PASS1

#include includes/PKCommon.glsl
#include includes/Reconstruction.glsl
#include includes/Noise.glsl

struct PassData
{
    sampler2DArray source;
    float2 offset;
    uint2 readwrite;
};

PK_DECLARE_RESTRICTED_READONLY_BUFFER(PassData, _AOPassParams);
uniform float3 _AOParams;

const float kNDotVBias = 0.002;
const float kGeometryAwareness = 50;
const int SAMPLE_COUNT = 16;
const float SAMPLE_RADII[SAMPLE_COUNT] =
{
    0.25f,
    0.3535533905932738f,
    0.4330127018922193f,
    0.5f,
    0.5590169943749474f,
    0.6123724356957945f,
    0.6614378277661476f,
    0.7071067811865475f,
    0.75f,
    0.7905694150420948f,
    0.82915619758885f,
    0.8660254037844386f,
    0.9013878188659973f,
    0.9354143466934853f,
    0.9682458365518542f,
    1.0f,
};

#define INTENSITY _AOParams.x
#define RADIUS _AOParams.y
#define TARGETSCALE _AOParams.z

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
    // @TODO solve this correctly later
    float d = 1.0f - (LinearizeDepth(tex2D(pk_ScreenDepth, uv).r) - pk_ProjectionParams.x) * pk_ProjectionParams.w;
    return d * d * d * d * d * d * d * d * d * d * d * d * d * d * d * d; //pow16
}

float3 GetSampleDirection(float2 uv, float3 normal, int index)
{
    float noiseg = NoiseGradient(uv * TARGETSCALE, pk_ScreenParams.xy);

    float noiseuv = fract(NoiseUV(0.0, index) + noiseg) * 2.0f - 1.0f;
    float theta = (NoiseUV(1.0, index) + noiseg) * PK_TWO_PI;

    float3 direction = float3(float2(cos(theta), sin(theta)) * sqrt(1.0f - noiseuv * noiseuv), noiseuv);

    direction *= SAMPLE_RADII[index] * RADIUS;

    return faceforward(direction, -normal, direction);
}

float ComputeAO(float3 P, float3 N, float3 S, float D)
{
    float3 V = S - P;
    float VdotV = dot(V, V) + 1e-4;
    float NdotV = max(dot(N, V) -kNDotVBias * D, 0.0f) * inversesqrt(VdotV);
    return NdotV * max(-VdotV / (RADIUS * RADIUS) + 1.0f, 0.0f);
}

float ComputeCoarseAO(float3 uvw, sampler2DArray source, float2 offset)
{
    float2 uv = uvw.xy;
    float3x3 projection = float3x3(pk_MATRIX_P);

    float vdepth = SampleDepth(uv);
    float3 vnormal = SampleViewSpaceNormal(uv);
    float3 vposition = ClipToViewPos(uv, vdepth);

    float AO = 0.0f;

    #pragma unroll SAMPLE_COUNT
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

    return 1.0f - pow(AO * INTENSITY / SAMPLE_COUNT, 0.6f);
}

half SeparableBlur(float3 uv00, sampler2DArray source, float2 offset)
{
    float3 delta = float3(offset * GetDepthBlurFactor(uv00.xy), 0);
    float3 uv10 = uv00 - delta;
    float3 uv11 = uv00 + delta;
    float3 uv20 = uv00 - delta * 2.0f;
    float3 uv21 = uv00 + delta * 2.0f;
    float3 uv30 = uv00 - delta * 3.2307692308f;
    float3 uv31 = uv00 + delta * 3.2307692308f;

    float3 n = SampleViewSpaceNormal(uv00.xy);

    float4 vdots0 = float4(dot(n, SampleViewSpaceNormal(uv10.xy)),
                           dot(n, SampleViewSpaceNormal(uv11.xy)),
                           dot(n, SampleViewSpaceNormal(uv20.xy)),
                           dot(n, SampleViewSpaceNormal(uv21.xy))) * 0.5f + 0.5f;
    float2 vdots1 = float2(dot(n, SampleViewSpaceNormal(uv30.xy)),
                           dot(n, SampleViewSpaceNormal(uv31.xy))) * 0.5f + 0.5f;

    float4 w0 = pow(vdots0, kGeometryAwareness.xxxx) * float4(0.31718061674f.xx, 0.19823788546f.xx);
    float3 w1 = float3(0.37004405286f, pow(vdots1, kGeometryAwareness.xx) * 0.11453744493f.xx);

    float4 s0 = float4(tex2D(source, uv10).r, tex2D(source, uv11).r, tex2D(source, uv20).r, tex2D(source, uv21).r);
    float3 s1 = float3(tex2D(source, uv00).r, tex2D(source, uv30).r, tex2D(source, uv31).r);

    return (dot(s0, w0) + dot(s1, w1)) / (dot(w0, 1.0f.xxxx) + dot(w1, 1.0f.xxx));
}

#if defined(AO_PASS0)
#define FUNC_FRAG ComputeCoarseAO
#elif defined(AO_PASS1)
#define FUNC_FRAG SeparableBlur
#endif

#pragma PROGRAM_VERTEX
layout(location = 0) in float4 in_POSITION0;
layout(location = 1) in float2 in_TEXCOORD0;
out float3 vs_TEXCOORD0;
out float2 vs_OFFSET;
out flat sampler2DArray vs_MainTex;

void main()
{
    PassData passdata = PK_BUFFER_DATA(_AOPassParams, gl_BaseInstance);
    vs_MainTex = passdata.source;
    vs_OFFSET = passdata.offset / textureSize(passdata.source, 0).xy;
    vs_TEXCOORD0 = float3(in_TEXCOORD0, passdata.readwrite.x);
    gl_Layer = int(passdata.readwrite.y);
    gl_Position = in_POSITION0;
};

#pragma PROGRAM_FRAGMENT
in float3 vs_TEXCOORD0;
in float2 vs_OFFSET;
in flat sampler2DArray vs_MainTex;
layout(location = 0) out float SV_Target0;
void main() { SV_Target0 = FUNC_FRAG(vs_TEXCOORD0, vs_MainTex, vs_OFFSET); };