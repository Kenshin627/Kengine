#version 460 core

layout(vertices = 4) out;

in VS_OUT
{
	vec3 normal;
	vec3 tangent;
	vec2 texcoord;
} tcs_in[];

out TCS_OUT
{
	vec3 normal;
	vec3 tangent;
	vec2 texcoord;
} tcs_out[];

uniform float tessLevel = 500.0;

void main()
{
	tcs_out[gl_InvocationID].normal = tcs_in[gl_InvocationID].normal;
	tcs_out[gl_InvocationID].tangent = tcs_in[gl_InvocationID].tangent;
	tcs_out[gl_InvocationID].texcoord = tcs_in[gl_InvocationID].texcoord;
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	if (gl_InvocationID == 0)
	{
		gl_TessLevelOuter[0] = tessLevel;
		gl_TessLevelOuter[1] = tessLevel;
		gl_TessLevelOuter[2] = tessLevel;
		gl_TessLevelOuter[3] = tessLevel;
		gl_TessLevelInner[0] = tessLevel;
		gl_TessLevelInner[1] = tessLevel;
	}
}