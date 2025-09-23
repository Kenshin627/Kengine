#version 460 core

uniform sampler2D diffuseMap;

in vec2 vTexcoord;

void main()
{
	float alpha = texture(diffuseMap, vTexcoord).r;
	if(alpha <= 0.0f)
	{
		discard;
	}
}