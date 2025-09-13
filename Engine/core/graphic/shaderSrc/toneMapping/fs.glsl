#version 460 core

in vec2			  vTexcoord;
uniform sampler2D screenMap;
uniform float     exposure;
out vec4		  FragColor;

void main()
{
	vec3 col = texture(screenMap, vTexcoord).rgb;
	vec3 mapped = vec3(1.0) - exp(-col * exposure);
	mapped = pow(col, vec3(1.0/2.2));
	FragColor = vec4(mapped, 1.0);
}