#pragma name Default

#pragma variable _Color
#pragma variable _Resolution
#pragma variable _Time

#pragma vertex
#version 330 core

layout(location = 0) in vec4 position;

void main()
{
	gl_Position = position;
};

#pragma fragment
#version 330 core

layout(location = 0) out vec4 color;

uniform float _Time;
uniform vec4 _Color;
uniform vec2 _Resolution;

vec3 hsv2rgb(vec3 c)
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
	float aspect = _Resolution.x / _Resolution.y;

	vec2 uv = gl_FragCoord.xy / _Resolution;

	uv.x -= 0.5;
	uv.x *= aspect;
	uv.x += 0.5;

	float cd = length(uv - 0.5) - _Time * 0.05;
	float c = step( mod(cd * 16.0, 1.0), 0.5);
	float h = mod(cd, 1.0);

	vec3 col = hsv2rgb(vec3(h, 1, 1));

	color.rgb = col * c;
	color.a = 1.0f;
};