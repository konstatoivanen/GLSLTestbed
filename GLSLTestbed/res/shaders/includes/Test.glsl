#ifndef TEST_GLSL
#define TEST_GLSL

#include PKCommon.glsl

struct m2s
{
	float tof;
	float st;
	float rt;
	float res;
	float2  uvLow;
	float2  uv;
	float2  a;
};

int ditherPattern[64] = int[]
(
	0,  32, 8,  40, 2,  34, 10, 42,
	48, 16, 56, 24, 50, 18, 58, 26,
	12, 44, 4,  36, 14, 46, 6,  38,
	60, 28, 52, 20, 62, 30, 54, 22,
	3,  35, 11, 43, 1,  33, 9,  41,
	51, 19, 59, 27, 49, 17, 57, 25,
	15, 47, 7,  39, 13, 45, 5,  37,
	63, 31, 55, 23, 61, 29, 53, 21
);

float ditherIndexValue(float2 pixelCoord)
{
	int x = int(mod(pixelCoord.x, 8));
	int y = int(mod(pixelCoord.y, 8));
	return ditherPattern[(x + y * 8)] / 65.0;
}

float dither(float color, float2 pixelCoord, int steps)
{
	float c = floor(color * steps) / steps;
	float f = ceil(color * steps) / steps;
	float d = ditherIndexValue(pixelCoord);
	float distance = abs(c - color) * steps;
	return (distance < d) ? c : f;
}

float3 hsv2rgb(float3 c)
{
	float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	float3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float2 rotate2D(float2 v, float rad) 
{
	float s = sin(rad);
	float c = cos(rad);

	float tx = v.x;
	float ty = v.y;

	return float2((c * tx) - (s * ty), (s * tx) + (c * ty));
}

float4 sampleImage(m2s i)
{
	float box = max(i.a.x, i.a.y) - i.tof;
	float boxm = min(i.a.x, i.a.y) - i.tof;
	float circle = length(i.uv - 0.5) - i.tof;

	float shape = mix(mix(box, circle, saturate(i.st * 2)), boxm, saturate((i.st * 2) - 1));

	float hue = mod(shape, 1.0);
	float cut = step(mod(shape * 16.0, 1.0), 0.4);
	float grad = dither(mod(shape * 3, 1.0) * cut, i.uvLow, 4);

	float opaqueclip = (sin(pk_Time.y * 0.0625) + 1) * 0.5;

	return float4(hsv2rgb(float3(hue, 1, grad)), mix(cut, grad, opaqueclip) );
}

#endif