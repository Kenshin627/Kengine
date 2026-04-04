#version 460 core

layout(local_size_x = 8, local_size_y = 8) in;

uniform sampler2D screenMap;
uniform float	  thresholdMin;
uniform float	  thresholdMax;

layout(binding = 0, rgba16f) uniform readonly image2D uSourceTex;
layout(binding = 1, rgba16f) uniform writeonly image2D uLDRTex;
layout(binding = 2, rgba16f) uniform writeonly image2D uHDRTex;

void main()
{
	ivec2 uv = ivec2(gl_GlobalInvocationID);
	vec3 color = imageLoad(uSourceTex, uv).rgb;
	
	float Y = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
	imageStore(uLDRTex, uv, vec4(color, 1.0));
	imageStore(uHDRTex, uv, vec4(color * 4.0 * smoothstep(thresholdMin, thresholdMax, Y), 1.0));
}