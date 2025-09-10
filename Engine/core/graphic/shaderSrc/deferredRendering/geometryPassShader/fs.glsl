#version 460 core

//GBUFFER
layout (location = 0) out vec3 gPosition; //position + depth in worldSpace
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gDiffuse;
layout (location = 3) out vec4 gSpecShiness;

in vec3 vPos;
//in vec3 vNormal;
in vec2 vTexcoord;
in mat3 vTBN;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;
uniform float     shiness;

//TODO: update by uniform or cameraBuffer
const float NEAR = 0.01;
const float FAR = 100;

void main()
{
	gPosition.xyz = vPos;
	//gNormal = normalize(vNormal);
	vec3 normal		 = texture(normalMap, vTexcoord).xyz;
	normal			 = normal * 2.0 - 1.0;
	gNormal			 = normalize(vTBN * normal);
	gDiffuse		 = texture(diffuseMap, vTexcoord).rgb;
	gSpecShiness.rgb = texture(specularMap, vTexcoord).rgb;
	gSpecShiness.a   = shiness;
}