#version 460 core

in vec2 gUv;
out vec4 FragColor;

uniform vec3 bladeTopColor;
uniform vec3 bladeBottomColor;

void main()
{
	vec3 col = mix(bladeBottomColor, bladeTopColor, gUv.y);
	FragColor = vec4(col, 1.0);
}