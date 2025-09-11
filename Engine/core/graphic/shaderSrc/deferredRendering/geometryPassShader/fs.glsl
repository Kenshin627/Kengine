#version 460 core

//GBUFFER
layout (location = 0) out vec3 gPosition; //position in viewPsace
layout (location = 1) out vec3 gNormal;   
layout (location = 2) out vec3 gDiffuse;
layout (location = 3) out vec4 gSpecShiness;

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexcoord;
in mat3 vTBN;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;
uniform sampler2D shinessMap;
uniform vec3	  diffuseColor;
uniform vec3	  specularColor;
uniform float     shiness;

uniform bool	  hasDiffTex;
uniform bool	  hasSpecTex;
uniform bool	  hasNormalTex;
uniform bool	  hasShinessTex;

void main()
{
	gPosition.xyz = vPos;
	if(hasNormalTex)
	{
		vec3 normal = texture(normalMap, vTexcoord).xyz;
		normal = normalize(normal * 2.0 - 1.0);
		gNormal = normalize(vTBN * normal);
	}
	else
	{
		gNormal = normalize(vNormal);
	}
	
	if(hasDiffTex)
	{
	
		gDiffuse = texture(diffuseMap, vTexcoord).rgb;
	}
	else
	{
		gDiffuse = diffuseColor;
	}

	if(hasSpecTex)
	{
		gSpecShiness.rgb = texture(specularMap, vTexcoord).rgb;
	}
	else
	{
		gSpecShiness.rgb = specularColor;
	}

	if(hasShinessTex)
	{
		gSpecShiness.a = texture(shinessMap, vTexcoord).r;
	}
	else
	{
		gSpecShiness.a = shiness;
	}
}