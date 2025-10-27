#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexcoord;
layout(location = 3) in vec3 aTangent;

out VS_OUT
{
	vec3 normal;
	vec3 tangent;
	vec2 texcoord;
} vs_out;

void main()
{
	vs_out.normal = aNormal;
	vs_out.tangent = aTangent;
	vs_out.texcoord = aTexcoord;
	gl_Position = vec4(aPos, 1.0);
}