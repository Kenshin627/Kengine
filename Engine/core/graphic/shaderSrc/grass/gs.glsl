#version 460 core

#define BLADESEGMENT 3
#define PI			 3.14159265359
#define TWOPI		 6.28318530718

uniform float bladeWidth;
uniform float bladeWidthRandom;
uniform float bladeHeight;
uniform float bladeHeightRandom;
uniform float bladeBendRandom;
uniform float bladeForward;
uniform float bladeCurve;

layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 viewProjectionMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec4 position;
	vec4 clipRange;
} cameraBuffer;

out vec2 gUv;

layout(points) in;
layout(triangle_strip, max_vertices = BLADESEGMENT * 2 + 1) out;

float rand(vec3 co)
{
	return fract(sin(dot(co.xyz, vec3(12.9898, 78.233, 53.539))) * 43758.5453);
}

mat3 AngleAxis3x3(float angle, vec3 axis)
{
	float c = cos(angle);
	float s = sin(angle);

	float t = 1 - c;
	float x = axis.x;
	float y = axis.y;
	float z = axis.z;

	return mat3
	(
		t * x * x + c, t * x * y - s * z, t * x * z + s * y,
		t * x * y + s * z, t * y * y + c, t * y * z - s * x,
		t * x * z - s * y, t * y * z + s * x, t * z * z + c
	);
}

in TES_OUT
{
	vec3 normal;
	vec3 tangent;
	vec2 texcoord;
} gs_in[];

void generateBladeVertex(vec3 pos, float width, float forward, float height, vec2 uv, mat3 matrix)
{
	vec3 localPos = pos + matrix * vec3(width, forward, height);
	gUv = uv;
	
	gl_Position = cameraBuffer.viewProjectionMatrix * vec4(localPos, 1.0);
	EmitVertex();
}

void main()
{
	vec4 pos = gl_in[0].gl_Position;
	vec3 n = normalize(gs_in[0].normal);
	vec3 t = normalize(gs_in[0].tangent);
	vec3 b = normalize(cross(n, t));
	mat3 TBN = mat3(t, b, n);
	mat3 FaceMatrix = AngleAxis3x3(rand(pos.xyz) * TWOPI, vec3(0, 0, 1));
	mat3 BendMatrix = AngleAxis3x3(rand(pos.zzx) * PI * 0.5 * bladeBendRandom, vec3(-1, 0, 0));
	mat3 transformMatrix = TBN * FaceMatrix * BendMatrix;
	mat3 transformFacingMatrix = TBN * FaceMatrix;
	float h = bladeHeight + (rand(pos.zyx) * 2.0 - 1.0) * bladeHeightRandom;
	float w = bladeWidth + (rand(pos.xzy) * 2.0 - 1.0) * bladeWidthRandom;
	float forward = rand(pos.yyz) * bladeForward;

	for(int i = 0; i < BLADESEGMENT; i++)
	{
		float t = i / BLADESEGMENT;
		float segentWidth = w * (1.0 - t);
		float segmentHeight = h * t;
		float segmentForward = pow(t, bladeCurve) * forward;
		mat3 transform = i == 0? transformFacingMatrix: transformMatrix;
		generateBladeVertex(pos.xyz, segentWidth, segmentForward, segmentHeight, vec2(0.0, t), transform);
		generateBladeVertex(pos.xyz, -segentWidth, segmentForward, segmentHeight, vec2(1.0, t), transform);
	}
	generateBladeVertex(pos.xyz, 0, forward, h, vec2(0.5, 1), transformMatrix);
	EndPrimitive();
}