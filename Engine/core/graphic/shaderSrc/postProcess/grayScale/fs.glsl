#version 460 core

out vec4 FragColor;

in vec2 vTexcoord;

uniform sampler2D screenMap;

void main()
{
	vec3 c = texture(screenMap, vTexcoord).rgb;
	float grayVal= 0.2126 * c.r + 0.7152 * c.g + 0.0722 * c.b;
	FragColor = vec4(vec3(grayVal), 1.0);
}