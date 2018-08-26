#pragma name HueTerraceSimple

#pragma variable _Color
#pragma variable _Resolution
#pragma variable _Time

#pragma vertex
#version 330 core

layout(location = 0) in vec4 position;
out vec2 texcoord;

void main()
{
	gl_Position = position;
	texcoord = (position.xy + 1) * 0.5;
};

#pragma fragment
#version 330 core
uniform float _Time;
uniform vec2  _Resolution;

layout(location = 0) out vec4 color;
in vec2 texcoord;

int ditherPattern[64] = int[]
(
	0, 32, 8, 40, 2, 34, 10, 42,
	48, 16, 56, 24, 50, 18, 58, 26,
	12, 44, 4, 36, 14, 46, 6, 38,
	60, 28, 52, 20, 62, 30, 54, 22,
	3, 35, 11, 43, 1, 33, 9, 41,
	51, 19, 59, 27, 49, 17, 57, 25,
	15, 47, 7, 39, 13, 45, 5, 37,
	63, 31, 55, 23, 61, 29, 53, 21
	);

float ditherIndexValue(vec2 pixelCoord)
{
	int x = int(mod(pixelCoord.x, 8));
	int y = int(mod(pixelCoord.y, 8));
	return ditherPattern[(x + y * 8)] / 65.0;
}
float dither(float color, vec2 pixelCoord, int steps)
{
	float c = floor(color * steps) / steps;
	float f = ceil(color * steps) / steps;
	float d = ditherIndexValue(pixelCoord);
	float distance = abs(c - color) * steps;
	return (distance < d) ? c : f;
}
float saturate(float v)
{
	return clamp(v, 0.0, 1.0);
}

vec3 hsv2rgb(vec3 c)
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
vec2 rotate2D(vec2 v, float rad)
{
	float s = sin(rad);
	float c = cos(rad);

	float tx = v.x;
	float ty = v.y;

	return vec2((c * tx) - (s * ty), (s * tx) + (c * ty));
}

void main()
{
	vec2 uv = texcoord;
	uv.x -= 0.5;
	uv.x *= (_Resolution.x / _Resolution.y);
	uv.x += 0.5;

	float  tof = _Time * 0.1;
	float  st = (sin(_Time * 0.5) + 1) * 0.5;
	float  rt = (sin(_Time * 0.25) + 1) * 0.5;
	float  res = mix(128, _Resolution.y, rt * rt);

	vec2 uvLow = round(uv * res);

	uv = uvLow / res;

	vec2 a = abs(rotate2D(uv - 0.5, tof * 2));

	float box = max(a.x, a.y) - tof;
	float boxm = min(a.x, a.y) - tof;
	float circle = length(uv - 0.5) - tof;

	float shape = mix(mix(box, circle, saturate(st * 2)), boxm, saturate((st * 2) - 1));

	float hue = mod(shape, 1.0);
	float cut = step(mod(shape * 16.0, 1.0), 0.5);
	float grad = dither(mod(shape * 3, 1.0) * cut, uvLow, 4);

	vec3 col = hsv2rgb(vec3(hue, 1, grad));

	color = vec4(col, 1.0);
};