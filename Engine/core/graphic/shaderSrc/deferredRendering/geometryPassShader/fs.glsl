#version 460 core

//GBUFFER
layout (location = 0) out vec3 gPosition; //position in viewPsace
layout (location = 1) out vec3 gNormal;   //normal in viewSpace 
layout (location = 2) out vec4 gDiffuse;  //diffsue or emissive + isEmissive
layout (location = 3) out vec4 gSpecShiness; //spec + shienss

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexcoord;
in mat3 vTBN;

uniform sampler2D diffuseMap;       //diffuse or emissive
uniform sampler2D specularMap;
uniform sampler2D normalMap;
uniform sampler2D shinessMap;
uniform vec3	  diffuseColor;
uniform vec3	  specularColor;
uniform vec3	  emissiveColor;
uniform float     shiness;

uniform bool	  hasDiffTex;
uniform bool	  hasSpecTex;
uniform bool	  hasNormalTex;
uniform bool	  hasShinessTex;
uniform bool      isEmissive;

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

	if(isEmissive)
	{
		gDiffuse.rgb = emissiveColor;
		gDiffuse.a = 1.0;
	}
	
	else if(hasDiffTex)
	{
	
		gDiffuse.rgb = texture(diffuseMap, vTexcoord).rgb;
		gDiffuse.a = 0.0;
	}
	else
	{
		gDiffuse.rgb = diffuseColor;
		gDiffuse.a = 0.0;
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