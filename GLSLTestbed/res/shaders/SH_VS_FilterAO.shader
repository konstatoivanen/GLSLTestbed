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

float CompareNormal(float3 normal0, float3 normal1)
{
    return pow((dot(normal0, normal1) + 1) * 0.5f, kGeometryAwareness);
}

float3 GetSampleDirection(float2 uv, float3 normal, float index)
{
    float noiseg = NoiseGradient(uv * TARGETSCALE, pk_ScreenParams.xy);

    float noiseuv = fract(NoiseUV(0.0, index) + noiseg) * 2.0f - 1.0f;
    float theta = (NoiseUV(1.0, index) + noiseg) * PK_TWO_PI;

    float3 direction = float3(float2(cos(theta), sin(theta)) * sqrt(1.0f - noiseuv * noiseuv), noiseuv);

    direction *= sqrt((index + 1) / SAMPLE_COUNT) * RADIUS;

    return faceforward(direction, -normal, direction);
}

float ComputeAO(float3 P, float3 N, float3 S, float D)
{
    float3 V = S - P;
    float VdotV = dot(V, V) + 1e-4;
    float NdotV = max(0, dot(N, V) - kNDotVBias * D);
    return NdotV / VdotV;
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

    return pow(AO * RADIUS * INTENSITY / SAMPLE_COUNT, 0.6f);
}

half SeparableBlur(float3 uvw, sampler2DArray source, float2 offset)
{
    float2 uv = uvw.xy;
    float2 delta = (offset / textureSize(source, 0).xy) * GetDepthBlurFactor(uv);

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

    float s = tex2D(source, uvw).r * w00;
    s += tex2D(source, float3(uv10, uvw.z)).r * w10;
    s += tex2D(source, float3(uv11, uvw.z)).r * w11;
    s += tex2D(source, float3(uv20, uvw.z)).r * w20;
    s += tex2D(source, float3(uv21, uvw.z)).r * w21;
    s += tex2D(source, float3(uv30, uvw.z)).r * w30;
    s += tex2D(source, float3(uv31, uvw.z)).r * w31;

    return s / (w00 + w10 + w11 + w20 + w21 + w30 + w31);
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
    vs_OFFSET = passdata.offset;
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