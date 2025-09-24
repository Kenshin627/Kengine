#version 460 core

uniform sampler2D screenMap;

in vec2 vTexcoord;

layout (location = 0) out vec4 FragLDRColor;
layout (location = 1) out vec4 FragHDRColor;

void main()
{
	vec3 color = texture(screenMap, vTexcoord).rgb;
	float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
	FragHDRColor = vec4(0.0, 0.0, 0.0, 1.0);
	FragLDRColor = vec4(color, 1.0);
	if(brightness > 1.0)
	{
		FragHDRColor = vec4(color, 1.0);
	}

	
}