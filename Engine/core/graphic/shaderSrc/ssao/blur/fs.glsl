#version 460 core

out vec4 FragColor;

in vec2 vTexcoord;

uniform uint blurRadius;
uniform sampler2D ssaoMap;

void main()
{
	vec2 texelSize = 1.0 / textureSize(ssaoMap, 0);
	float val = 0.0;
	//4x4 blur
	for(int i = 0; i < blurRadius; i++)
	{
		for(int j = 0; j < blurRadius; j++)
		{
			val += texture(ssaoMap, vTexcoord + vec2(i, j) * texelSize).r;
		}
	}
	val /= float(blurRadius * blurRadius);
	FragColor = vec4(vec3(val), 1.0);
}