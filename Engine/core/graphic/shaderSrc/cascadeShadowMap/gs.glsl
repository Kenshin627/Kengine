#version 460 core

layout(triangles, invocations = 5) in;
layout(triangle_strip, max_vertices = 3) out;

layout (std140, binding = 2) uniform LightMatricesBuffer
{
	mat4 lightMatrices[16];
} lightMatricesBuffer;

void main()
{
	for(int i = 0; i < 3; i++)
	{
		gl_Position = lightMatricesBuffer.lightMatrices[gl_InvocationID] * gl_in[i].gl_Position;
		gl_Layer = gl_InvocationID;
		EmitVertex();
	}
	EndPrimitive();
}