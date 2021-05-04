#pragma once
#include HLSLSupport.glsl

#define HISTOGRAM_THREAD_COUNT 16
#define NUM_HISTOGRAM_BINS 256
#define EPSILON 0.0001

PK_DECLARE_CBUFFER(pk_TonemappingParams)
{
    float pk_MinLogLuminance;
    float pk_InvLogLuminanceRange;
    float pk_LogLuminanceRange;
    float pk_TargetExposure;
    float pk_AutoExposureSpeed;
    float pk_BloomIntensity;
    float pk_BloomDirtIntensity;
    float pk_Saturation;
    sampler2D pk_BloomLensDirtTex;
    sampler2D pk_HDRScreenTex;
};

PK_DECLARE_BUFFER(uint, pk_Histogram);

#define LOG_LUMINANCE_MIN pk_MinLogLuminance
#define LOG_LUMINANCE_INV_RANGE pk_InvLogLuminanceRange
#define LOG_LUMINANCE_RANGE pk_LogLuminanceRange
#define TARGET_EXPOSURE pk_TargetExposure
#define EXPOSURE_ADJUST_SPEED pk_AutoExposureSpeed

float GetAutoExposure()
{
    return uintBitsToFloat(PK_BUFFER_DATA(pk_Histogram, 256));
}

void SetAutoExposure(float exposure)
{
    PK_BUFFER_DATA(pk_Histogram, 256) = floatBitsToUint(exposure);
}

// https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/course-notes-moving-frostbite-to-pbr-v2.pdf
float computeEV100( float aperture, float shutterTime, float ISO)
{
    // EV number is defined as:
    // 2^ EV_s = N^2 / t and EV_s = EV_100 + log2 (S /100)
    // This gives
    // EV_s = log2 (N^2 / t)
    // EV_100 + log2 (S /100) = log2 (N^2 / t)
    // EV_100 = log2 (N^2 / t) - log2 (S /100)
    // EV_100 = log2 (N^2 / t . 100 / S)
    return log2(pow2(aperture) / shutterTime * 100 / ISO);
}

float computeEV100FromAvgLuminance( float avgLuminance)
{
    // We later use the middle gray at 12.7% in order to have
    // a middle gray at 18% with a sqrt (2) room for specular highlights
    // But here we deal with the spot meter measuring the middle gray
    // which is fixed at 12.5 for matching standard camera
    // constructor settings (i.e. calibration constant K = 12.5)
    // Reference : http://en.wikipedia.org/wiki/Film_speed
    return log2(avgLuminance * 100.0f / 12.5f);
}

float convertEV100ToExposure( float EV100)
{
    // Compute the maximum luminance possible with H_sbs sensitivity
    // maxLum = 78 / ( S * q ) * N^2 / t
    // = 78 / ( S * q ) * 2^ EV_100
    // = 78 / (100 * 0.65) * 2^ EV_100
    // = 1.2 * 2^ EV
    // Reference : http://en.wikipedia.org/wiki/Film_speed
    float maxLuminance = 1.2f * pow(2.0f , EV100);
    return 1.0f / maxLuminance;
}

float3 TonemapHejlDawson(half3 color, float exposure)
{
	const half a = 6.2;
	const half b = 0.5;
	const half c = 1.7;
	const half d = 0.06;

	color *= exposure;
	color = max(float3(0.0), color - 0.004);
	color = (color * (a * color + b)) / (color * (a * color + c) + d);
	return color * color;
}

float3 Saturation(float3 color, float amount) 
{
	float grayscale = dot(color, float3(0.3, 0.59, 0.11));
	return lerp_true(grayscale.xxx, color, 0.8f);
}

float3 LinearToGamma(float3 color)
{
	//Source: http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html?m=1
	float3 S1 = sqrt(color);
	float3 S2 = sqrt(S1);
	float3 S3 = sqrt(S2);
	return 0.662002687 * S1 + 0.684122060 * S2 - 0.323583601 * S3 - 0.0225411470 * color;
}
