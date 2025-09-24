#version 460 core

uniform sampler2D diffuseMap;

in vec2 vTexcoord;

void main()
{
	float alpha = texture(diffuseMap, vTexcoord).a;
	if(alpha < 0.1f)
	{
		discard;
	}
}