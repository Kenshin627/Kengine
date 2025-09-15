#version 460 core

//GBUFFER
layout (location = 0) out vec3  gPosition;    //position in viewPsace
layout (location = 1) out vec3  gNormal;      //normal in viewSpace 
layout (location = 2) out vec4  gDiffuse;     //diffsue or emissive + isEmissive or albedo
layout (location = 3) out vec4  gSpecShiness; //spec + shienss  or metallic + roughness
layout (location = 4) out float gMaterialType;//material type blinnphong 0  pbr 1

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexcoord;
in mat3 vTBN;

uniform sampler2D diffuseMap;       //diffuse or albedo
uniform sampler2D specularMap;      //spec or metallic
uniform sampler2D normalMap;        //normal
uniform sampler2D shinessMap;      // shiness or roughness
uniform int		  materialType;

//////////////////BLINN PHONG////////////////////////////
uniform vec3	  diffuseColor;
uniform vec3	  specularColor;
uniform vec3	  emissiveColor;
uniform float     shiness;
uniform bool	  hasDiffTex;
uniform bool	  hasSpecTex;
uniform bool	  hasNormalTex;
uniform bool	  hasShinessTex;
uniform bool      isEmissive;
///////////////////////////////////////////////////////

/////////////////PBR///////////////////////////////////
uniform vec3	  albedoColor;
uniform float	  metallic;
uniform float	  roughness;
uniform bool	  hasAlbedoTex;
uniform bool	  hasMetallicTex;
uniform bool	  hasRoughnessTex;
//////////////////////////////////////////////////////


void main()
{
	gPosition.xyz = vPos;
	gMaterialType = materialType;
	if(hasNormalTex)
	{
		vec3 normal = texture(normalMap, vTexcoord).xyz;
		normal = normal * 2.0 - 1.0;
		gNormal = normalize(vTBN * normal);
	}
	else
	{
		gNormal = normalize(vNormal);
	}

	//blinnphong
	if(materialType == 0)
	{
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
	//pbr
	else if(materialType == 1)
	{
		if(hasAlbedoTex)
		{
			gDiffuse.rgb = texture(diffuseMap, vTexcoord).rgb;
		}
		else
		{
			gDiffuse.rgb = albedoColor;
		}

		if(hasMetallicTex)
		{
			gSpecShiness.r = texture(specularMap, vTexcoord).r;
		}
		else
		{
			gSpecShiness.r = metallic;
		}

		if(hasRoughnessTex)
		{
			gSpecShiness.g = texture(shinessMap, vTexcoord).r;
		}
		else
		{
			gSpecShiness.g = roughness;
		}
	}
}