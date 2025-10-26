#version 460 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 vPos[];

out vec3 gPos;
out vec3 gNormal;
out vec2 baryCentric;

vec3 calcFaceNormal()
{
	vec3 edge0 = normalize(vPos[1] - vPos[0]);
	vec3 edge1 = normalize(vPos[2] - vPos[0]);
	vec3 normal = cross(edge0, edge1);
	return normalize(normal);
}

void main()
{
	vec3 n = calcFaceNormal();
	gl_Position = gl_in[0].gl_Position;
	baryCentric = vec2(1, 0);
	gNormal = n;
	gPos = vPos[0];
	EmitVertex();

	gl_Position = gl_in[1].gl_Position;
	baryCentric = vec2(0, 1);
	gNormal = n;
	gPos = vPos[1];
	EmitVertex();

	gl_Position = gl_in[2].gl_Position;
	baryCentric = vec2(0, 0);
	gPos = vPos[2];
	gNormal = n;
	EmitVertex();

	EndPrimitive();
}