#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexcoord;
layout (location = 3) in vec3 aTangent;

layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 viewProjectionMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec4 position;
	vec4 clipRange;
} cameraBuffer;

uniform mat4 modelMatrix;
uniform mat4 modelMatrixInvertTranspose;

uniform bool hasNormalTex;
uniform bool hasHeightTex;

out vec3 vPos;
out vec3 vNormal;
out vec2 vTexcoord;
out mat3 vTBN;

uniform bool      enablePOM;

//only has value in heightMap
out vec3 tangentSpaceViewPos;
out vec3 tangentSpaceFragPos;

void main()
{	
	mat4 modelViewMatrix = cameraBuffer.viewMatrix* modelMatrix;
	mat4 modelViewMatrixInverseTranspose = transpose(inverse(modelViewMatrix));
	if(hasHeightTex)
	{
		vec3 pos = vec3(modelMatrix * vec4(aPos, 1.0));
		vec3 n = normalize(mat3(modelMatrix) * aNormal);
		vec3 t = mat3(modelMatrix) * aTangent;
		t = normalize(t - n * dot(t, n));
		vec3 b = cross(n, t);
		mat3 tbn = transpose(mat3(t, b, n));
		if(enablePOM)
		{
			tangentSpaceViewPos = tbn * cameraBuffer.position.xyz;
			tangentSpaceFragPos = tbn * pos;
		}
	}

	if(hasNormalTex)
	{
		//mat3 modelMat3 = mat3(modelMatrix);
		vec3 t = mat3(modelViewMatrixInverseTranspose) * aTangent;
		vec3 n = normalize(mat3(modelViewMatrixInverseTranspose) * aNormal);
		t = normalize(t - n * dot(t, n));
		vec3 b = cross(n, t);
		vTBN = mat3(t, b, n);
	}
	else
	{
		
		vNormal = normalize((vec3(modelViewMatrixInverseTranspose * vec4(aNormal, 0.0))));
	}
	vec4 viewPos = modelViewMatrix * vec4(aPos, 1.0);
	vPos = viewPos.xyz / viewPos.w;
	vTexcoord = aTexcoord;
	gl_Position =  cameraBuffer.projectionMatrix * viewPos;
}