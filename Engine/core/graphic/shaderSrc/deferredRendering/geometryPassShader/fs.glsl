#version 460 core

//GBUFFER
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gDiffuseSpec;

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexcoord;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform float     shiness;

void main()
{
	gPosition = vPos;
	gNormal = normalize(vNormal);
	vec3 diff = texture(diffuseMap, vTexcoord).rgb;
	float spec = texture(specularMap, vTexcoord).r;
	gDiffuseSpec = vec4(diff, spec);
}