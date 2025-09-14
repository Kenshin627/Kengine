#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexcoord;

out vec2 vTexcoord;

void main()
{
	vTexcoord = aTexcoord;
	gl_Position = vec4(aPos, 1.0, 1.0);
}