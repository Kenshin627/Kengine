#version 460 core

in vec2			  vTexcoord;
uniform sampler2D gaussianBlurMap;
uniform sampler2D ldrMap;
uniform float     exposure;
out vec4		  FragColor;

void main()
{
	vec3 hdr = texture(gaussianBlurMap, vTexcoord).rgb;
	vec3 ldr = texture(ldrMap, vTexcoord).rgb;
	vec3 col = hdr + ldr;
	//vec3 mapped = vec3(1.0) - exp(-col * exposure);
	vec3 mapped = pow(col, vec3(0.4545));
	FragColor = vec4(mapped, 1.0);
}