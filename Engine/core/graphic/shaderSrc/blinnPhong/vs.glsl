#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexcoord;
layout (location = 3) in vec3 aTangent;
layout (location = 3) in vec3 aBiTangent;

uniform mat4 modelMatrix;
uniform mat4 modelMatrixInvertTranspose;

layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 viewProjectionMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec3 position;
} cameraBuffer;

out vec3 vPos;
out vec3 vNormal;
out vec2 vTexcoord;


void main()
{
	vec4 worldPos = modelMatrix * vec4(aPos, 1.0);
	vPos = worldPos.xyz / worldPos.w;
	vNormal = normalize((vec3(modelMatrixInvertTranspose * vec4(aNormal, 0.0))));
	vTexcoord = aTexcoord;
	gl_Position =  cameraBuffer.viewProjectionMatrix * worldPos;
}