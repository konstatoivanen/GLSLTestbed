#pragma name Default
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

uniform vec4 _Color;

void main()
{
	vec2 uv = gl_FragCoord.xy / 512;

	float res = _Color.r * _Color.r * 512.0;

	uv = floor(uv * res) / res;

	color = vec4(0, uv.x, uv.y, 1);
};