#version 460 core

//GBUFFER
layout (location = 0) out vec4 gPosition; //position + depth in worldSpace
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gDiffuse;
layout (location = 3) out vec4 gSpecShiness;

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexcoord;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform float     shiness;

//TODO: update by uniform or cameraBuffer
const float NEAR = 0.01;
const float FAR = 100;

float depthToLinear(float depth)
{
	float z = depth * 2.0 - 1.0; // »Øµ½NDC
    return (2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));    
}

void main()
{
	gPosition.xyz = vPos;
	gPosition.w = depthToLinear(gl_FragCoord.z);
	gNormal = normalize(vNormal);
	gDiffuse = texture(diffuseMap, vTexcoord).rgb;
	gSpecShiness.rgb = texture(specularMap, vTexcoord).rgb;
	gSpecShiness.a = shiness;
}