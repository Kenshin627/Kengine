#version 460 core

in vec2			  vTexcoord;
layout ( binding = 0 ) uniform sampler2D lDRMap;
layout ( binding = 1 ) uniform sampler2D hDRBlurMap;
uniform bool	  enableBloom;
uniform float     exposure;
out vec4		  FragColor;

void main()
{
	vec3 hdr = vec3(0.0);
	if(enableBloom)
	{
		hdr = texture(hDRBlurMap, vTexcoord).rgb;
	}
	vec3 ldr = texture(lDRMap, vTexcoord).rgb;
	vec3 col = hdr + ldr;
	vec3 mapped = vec3(1.0) - exp(-col * exposure);
	mapped = pow(col, vec3(0.4545));
	FragColor = vec4(mapped, 1.0);
}