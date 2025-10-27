#version 460 core

layout(quads, point_mode, fractional_odd_spacing, ccw) in;

in TCS_OUT
{
	vec3 normal;
	vec3 tangent;
	vec2 texcoord;
} tes_in[];

out TES_OUT
{
	vec3 normal;
	vec3 tangent;
	vec2 texcoord;
} tes_out;

void main()
{
	tes_out.normal = tes_in[0].normal;
	tes_out.tangent = tes_in[0].tangent;
	tes_out.texcoord = tes_in[0].texcoord;
	vec3 bottom = mix(gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz, gl_TessCoord.x);
	vec3 top = mix(gl_in[3].gl_Position.xyz, gl_in[2].gl_Position.xyz, gl_TessCoord.x);
	gl_Position = vec4(mix(bottom, top, gl_TessCoord.y), 1.0);
}