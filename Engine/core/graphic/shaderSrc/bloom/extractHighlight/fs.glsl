#version 460 core

uniform sampler2D screenMap;
uniform float thresholdMin;
uniform float thresholdMax;

in vec2 vTexcoord;

layout (location = 0) out vec4 FragHDRColor;
layout (location = 1) out vec4 FragLDRColor;

void main()
{
	vec3 color = texture(screenMap, vTexcoord).rgb;
	float Y = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
	FragLDRColor = vec4(color, 1.0);
	FragHDRColor = vec4(color * 4.0 * smoothstep(thresholdMin, thresholdMax, Y), 1.0);
}