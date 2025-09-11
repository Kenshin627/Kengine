#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexcoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBiTangent;

layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 viewProjectionMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec3 position;
} cameraBuffer;

uniform mat4 modelMatrix;
uniform mat4 modelMatrixInvertTranspose;

uniform bool hasNormalTex;

out vec3 vPos;
out vec3 vNormal;
out vec2 vTexcoord;
out mat3 vTBN;

void main()
{	
	mat4 modelViewMatrix = cameraBuffer.viewMatrix* modelMatrix;
	if(hasNormalTex)
	{
		mat3 modelMat3 = mat3(modelMatrix);
		vec3 t = mat3(modelMatrix) * aTangent;
		vec3 n = normalize(mat3(modelMatrix) * aNormal);
		t = normalize(t - n * dot(t, n));
		vec3 b = cross(n, t);
		vTBN = mat3(t, b, n);
	}
	else
	{
		mat4 modelViewMatrixInverseTranspose = transpose(inverse(modelViewMatrix));
		vNormal = normalize((vec3(modelViewMatrixInverseTranspose * vec4(aNormal, 0.0))));
	}
	vec4 viewPos = modelViewMatrix * vec4(aPos, 1.0);
	vPos = viewPos.xyz / viewPos.w;
	vTexcoord = aTexcoord;
	gl_Position =  cameraBuffer.projectionMatrix * viewPos;
}