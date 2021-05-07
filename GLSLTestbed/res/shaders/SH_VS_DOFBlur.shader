// Based on: https://github.com/keijiro/KinoBokeh
//
// Kino/Bokeh - Depth of field effect
//
// Copyright (C) 2016 Unity Technologies
// Copyright (C) 2015 Keijiro Takahashi
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
#extension GL_ARB_shader_viewport_layer_array : require
#multi_compile PASS_PREFILTER PASS_DISKBLUR

#include includes/DepthOfFieldShared.glsl

#if defined(PASS_PREFILTER)
    uniform sampler2D _MainTex;
#else
    uniform sampler2DArray _MainTex;
#endif

#define _MainTex_TexelSize (1.0f / textureSize(_MainTex, 0).xy)

#pragma PROGRAM_VERTEX
layout(location = 0) in float4 in_POSITION0;
layout(location = 1) in float2 in_TEXCOORD0;

#if defined(PASS_PREFILTER)
    out float2 vs_TEXCOORDS[4];
#else 
    out float3 vs_TEXCOORDS[SAMPLE_COUNT + 1];
#endif

void main()
{
	gl_Position = in_POSITION0;

#if defined(PASS_PREFILTER)
    gl_Layer = 0;

    float3 offsets = _MainTex_TexelSize.xyx * float3(0.5, 0.5, -0.5);
    vs_TEXCOORDS[0] = in_TEXCOORD0 - offsets.xy;
    vs_TEXCOORDS[1] = in_TEXCOORD0 - offsets.zy;
    vs_TEXCOORDS[2] = in_TEXCOORD0 + offsets.zy;
    vs_TEXCOORDS[3] = in_TEXCOORD0 + offsets.xy;
#else 
    gl_Layer = 1;

    float margin = _MainTex_TexelSize.y * 2;
	vs_TEXCOORDS[0] = float3(in_TEXCOORD0, 1.0f / margin);

    for (uint i = 0; i < SAMPLE_COUNT; ++i)
    {
        float2 disp = SAMPLE_KERNEL[i] * pk_MaximumCoC;
        float dist = length(disp);
        disp.x *= pk_ScreenParams.y / pk_ScreenParams.x;
        vs_TEXCOORDS[i + 1] = float3(in_TEXCOORD0 + disp, dist - margin);
    }
#endif
};

#pragma PROGRAM_FRAGMENT

#if defined(PASS_PREFILTER)
    in float2 vs_TEXCOORDS[4];
#else 
    in float3 vs_TEXCOORDS[SAMPLE_COUNT + 1];
#endif

layout(location = 0) out float4 SV_Target0;

void main()
{
    #if defined(PASS_PREFILTER)
        float4 depths = float4(SampleLinearDepth(vs_TEXCOORDS[0]),
                               SampleLinearDepth(vs_TEXCOORDS[1]),
                               SampleLinearDepth(vs_TEXCOORDS[2]),
                               SampleLinearDepth(vs_TEXCOORDS[3]));

        float4 cocs = GetCirclesOfConfusion(depths);
        float4 weights = saturate(abs(cocs) / pk_MaximumCoC);

        float3 average  = tex2D(_MainTex, vs_TEXCOORDS[0]).rgb * weights.x;
               average += tex2D(_MainTex, vs_TEXCOORDS[1]).rgb * weights.y; 
               average += tex2D(_MainTex, vs_TEXCOORDS[2]).rgb * weights.z;
               average += tex2D(_MainTex, vs_TEXCOORDS[3]).rgb * weights.w;
               average /= dot(weights, float4(1.0f));

        SV_Target0 = float4(average, dot(cocs, float4(0.25f)));
    #else
        float4 center = tex2D(_MainTex, float3(vs_TEXCOORDS[0].xy, 0));
        float4 background = float4(0.0f); 
        float4 foreground = float4(0.0f); 

        #pragma unroll SAMPLE_COUNT
        for (uint i = 1; i <= SAMPLE_COUNT; ++i)
        {
            float4 value = tex2D(_MainTex, float3(vs_TEXCOORDS[i].xy, 0));
            float backgroundCoC = max(min(center.a, value.a), 0);
            background += float4(value.rgb, 1.0f) * saturate((backgroundCoC - vs_TEXCOORDS[i].z) * vs_TEXCOORDS[0].z);
            foreground += float4(value.rgb, 1.0f) * saturate((-value.a - vs_TEXCOORDS[i].z) * vs_TEXCOORDS[0].z);
        }

        background.rgb /= background.a + (background.a < 1e-4f ? 1.0f : 0.0f);
        foreground.rgb /= foreground.a + (foreground.a < 1e-4f ? 1.0f : 0.0f);

        background.a = smoothstep(_MainTex_TexelSize.y, _MainTex_TexelSize.y * 2, center.a);
        foreground.a = saturate(foreground.a * PK_PI / SAMPLE_COUNT);

        float3 color = lerp(background.rgb * background.a, foreground.rgb, foreground.a);

	    SV_Target0 = float4(color, (1.0f - background.a) * (1.0f - foreground.a));
    #endif
};

