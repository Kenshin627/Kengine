#version 460 core

#define MAX_HALF_KERNEL_SIZE 21

in vec2 vTexcoord;
out vec4 FragColor;

uniform sampler2D screenMap;
uniform int isHorizontal;
uniform float kernels[MAX_HALF_KERNEL_SIZE];

uniform float scale;
uniform float strength;

void main()
{
	vec2 texelSize = 1.0 / textureSize(screenMap, 0); // gets size of single texel
    vec3 result = texture(screenMap, vTexcoord).rgb * kernels[0]; // current fragment's contribution
    vec2 offset;
    float k;
    float t;
    for(int i = 1; i < MAX_HALF_KERNEL_SIZE; i += 2)
    {
        k = kernels[i] + kernels[i + 1];
        t = kernels[i + 1] / k;
        vec2 direction = vec2(isHorizontal, 1 - isHorizontal);
        offset = texelSize * direction * (float(i) + t) * scale;
        result += texture(screenMap, vTexcoord + offset).rgb * k * strength;
        result += texture(screenMap, vTexcoord - offset).rgb * k * strength;
    }
    FragColor = vec4(result, 1.0);
}