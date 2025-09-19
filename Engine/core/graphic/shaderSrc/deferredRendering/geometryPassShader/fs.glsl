#version 460 core
#define MIN_STEPS 32.0
#define MAX_STEPS 64.0

//GBUFFER
layout (location = 0) out vec3  gPosition;     //position in viewPsace
layout (location = 1) out vec3  gNormal;       //normal in viewSpace 
layout (location = 2) out vec4  gDiffuse;      //diffsue or emissive + isEmissive or albedo
layout (location = 3) out vec4  gSpecShiness;  //spec + shienss  or metallic + roughness
layout (location = 4) out float gMaterialType; //material type blinnphong 0  pbr 1

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexcoord;
in mat3 vTBN;

//only has value in heightMap
in vec3 tangentSpaceViewPos;
in vec3 tangentSpaceFragPos;

uniform sampler2D diffuseMap;       //diffuse or albedo
uniform sampler2D specularMap;      //spec or metallic
uniform sampler2D normalMap;        //normal
uniform sampler2D shinessMap;       //shiness or roughness
uniform sampler2D heightMap;        //displace mapping
uniform int		  materialType; 
uniform bool	  hasHeightTex;
uniform float	  heightMapScale = 0.05;

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

vec2 ParallaxOcclusionMap(vec2 texCoord, vec3 tangentSpaceViewDir)
{
	float numLayers         = mix(MIN_STEPS, MAX_STEPS, max(dot(vec3(0,0,1), tangentSpaceViewDir), 0.0));
	float deltaDepth        = 1.0 / numLayers;
	float currentLayerDepth = 0.0;
	vec2  currentTexcoord   = texCoord;
	//load heightmap need covert to displace map
	float currentMapDepth   = 1.0 - texture(heightMap, currentTexcoord).r;
	vec2 deltaTexcoord = tangentSpaceViewDir.xy / tangentSpaceViewDir.z * heightMapScale;
	deltaTexcoord /= numLayers;
	
	while(currentLayerDepth < currentMapDepth)
	{
		currentLayerDepth += deltaDepth;
		currentTexcoord   -= deltaTexcoord;
		currentMapDepth    = 1.0 - texture(heightMap, currentTexcoord).r;
	}
	vec2 lastStepTexcoord = currentTexcoord + deltaTexcoord;
	float currentDepthRatio = currentMapDepth - currentLayerDepth;
	float lastDepthRatio = 1.0 - texture(heightMap, lastStepTexcoord).r - currentLayerDepth + deltaDepth;
	float weight = currentDepthRatio / (currentDepthRatio - lastDepthRatio);
	return (1.0 - weight) * currentTexcoord + weight * lastStepTexcoord;
}

void main()
{
	gPosition.xyz = vPos;
	gMaterialType = materialType;
	vec2 texCoord = vTexcoord;
	if(hasHeightTex)
	{
		vec3 tangentSpaceViewDir = normalize(tangentSpaceViewPos - tangentSpaceFragPos);
		texCoord = ParallaxOcclusionMap(vTexcoord, tangentSpaceViewDir);
	}

	if(hasNormalTex)
	{
		vec3 normal = texture(normalMap, texCoord).xyz;
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
		
			gDiffuse.rgb = texture(diffuseMap, texCoord).rgb;
			gDiffuse.a = 0.0;
		}
		else
		{
			gDiffuse.rgb = diffuseColor;
			gDiffuse.a = 0.0;
		}

		if(hasSpecTex)
		{
			gSpecShiness.rgb = texture(specularMap, texCoord).rgb;
		}
		else
		{
			gSpecShiness.rgb = specularColor;
		}

		if(hasShinessTex)
		{
			gSpecShiness.a = texture(shinessMap, texCoord).r;
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
			gDiffuse.rgb = texture(diffuseMap, texCoord).rgb;
		}
		else
		{
			gDiffuse.rgb = albedoColor;
		}

		if(hasMetallicTex)
		{
			gSpecShiness.r = texture(specularMap, texCoord).r;
		}
		else
		{
			gSpecShiness.r = metallic;
		}

		if(hasRoughnessTex)
		{
			gSpecShiness.g = texture(shinessMap, texCoord).r;
		}
		else
		{
			gSpecShiness.g = roughness;
		}
	}
}