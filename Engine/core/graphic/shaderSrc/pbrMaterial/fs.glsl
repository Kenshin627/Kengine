#version 460 core
#define MAX_LIGHT_COUNT 16
#define PI 3.14159265358979

layout (location = 0) out vec4 FragColor;

in vec2 vTexcoord;
in vec3 vPos;
in vec3 vNormal;
in mat3 vTBN;

//only has value in heightMap
in vec3 tangentSpaceViewPos;
in vec3 tangentSpaceFragPos;

struct Light
{
	vec4	position;
	vec4	direction;
	vec4	color;
	vec4	attentionFactor;
	float	outterCutoff;
	float	innerCutoff;
	int		type;
	int		lightCount;
};


uniform sampler2D diffuseMap;       //diffuse or albedo
uniform sampler2D specularMap;      //spec or metallic
uniform sampler2D normalMap;        //normal
uniform sampler2D shinessMap;       //shiness or roughness
uniform sampler2D heightMap;        //displace mapping
uniform bool	  hasHeightTex;
uniform float	  pomScale;
uniform bool      enablePOM;

uniform vec3	  albedoColor;
uniform float	  metallic;
uniform float	  roughness;
uniform bool	  hasAlbedoTex;
uniform bool	  hasMetallicTex;
uniform bool	  hasRoughnessTex;
uniform bool	  hasNormalTex;

//cascaded shadow map
uniform bool		   enableCSM;
uniform sampler2DArray cascadedShadowMap;
uniform int			   cascadedLayerCount;
uniform float          cascadedLayerDistances[16];
uniform int            cascadedShadowLightIndex;   //index to lightBuffer ubo
uniform int 		   pcfSize;
uniform bool		   displayCacadedColor;
uniform bool		   enablePCF;
 
layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 viewProjectionMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec4 position;
	vec4 clipRange;
} cameraBuffer;

layout (std140, binding = 1) uniform LightBuffer
{
	Light lights[MAX_LIGHT_COUNT];
} lightBuffer;

layout (std140, binding = 2) uniform LightMatricesBuffer
{
	mat4 lightMatrices[16];
} lightMatricesBuffer;


float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float calcShadow(vec3 viewSpacePos, vec3 viewSpaceNormal, out int layer)
{	
	//select cascaded shadowmap layer depends on viewspace Fragpos distance 
	float depth = abs(viewSpacePos.z);
	layer = -1;
	for(int i = 0; i < cascadedLayerCount; i++)
	{
		if(depth < cascadedLayerDistances[i])
		{
			layer = i;
			break;
		}
	}
	if(layer == -1)
	{
		layer = cascadedLayerCount;
	}

	//calc fragPos worldPos, transform to current Cascaded lightMatrices
	vec4 worldPos = inverse(cameraBuffer.viewMatrix) * vec4(viewSpacePos, 1.0);
	worldPos /= worldPos.w;
	mat4 currentLayerLightMatrix = lightMatricesBuffer.lightMatrices[layer];
	vec4 lightSpacePos = currentLayerLightMatrix * worldPos;
	lightSpacePos /= lightSpacePos.w;
	lightSpacePos = lightSpacePos * 0.5 + 0.5;
	float currentDepth = lightSpacePos.z;
	if(currentDepth > 1.0)
	{
		currentDepth = 0.0f;
	}

	vec4 shadowLightDir = lightBuffer.lights[cascadedShadowLightIndex].direction;
	//convert direction to viewSpace, bcaz wll dot with normal in viewSpace
	vec4 shadowLightDirInViewSpace = cameraBuffer.viewMatrix * vec4(vec3(-shadowLightDir), 0.0);
	shadowLightDirInViewSpace /= shadowLightDirInViewSpace.w;
	vec3 shadowlightDir =  normalize(shadowLightDirInViewSpace.xyz);
	//float bias = dot(viewSpaceNormal, shadowlightDir);
	float bias = max(0.05 * (1.0 - dot(viewSpaceNormal, shadowlightDir)), 0.005);
    const float biasModifier = 0.5f;
	float far = cameraBuffer.clipRange[1];
    if (layer == cascadedLayerCount)
    {
        bias *= 1 / (far * biasModifier);
    }
    else
    {
        bias *= 1 / (cascadedLayerDistances[layer] * biasModifier);
    }

	float shadow = 0.0f;
	vec2 texelSize = 1.0 / vec2(textureSize(cascadedShadowMap, 0));
	int count = 0;
	int pcfBlurSize = 0;
	if(enablePCF)
	{
		pcfBlurSize = pcfSize;
	}
	for(int x  = -pcfBlurSize; x <= pcfBlurSize; x++)
	{
		for(int y = -pcfBlurSize; y <= pcfBlurSize; y++)
		{
			vec2 texCoord = lightSpacePos.xy + vec2(x, y) * texelSize;
			float pcfShadowDepth = texture(cascadedShadowMap, vec3(texCoord, layer)).r;
			if((currentDepth - bias) > pcfShadowDepth)
			{
				shadow += 1.0;
			}
			count++;
		}
	}
	shadow /= count;
	return shadow;
}

void main()
{
	//calc illumulation in worldSpace
	vec3 fragmentPos = vPos;
	vec3 n;
	if(hasNormalTex)
	{
		vec3 normal = texture(normalMap, vTexcoord).xyz;
		normal = normal * 2.0 - 1.0;
		n = normalize(vTBN * normal);
	}
	else
	{
		n = normalize(vNormal);
	}
	vec3  v = normalize(cameraBuffer.position.xyz - fragmentPos);
	float ambientOcclusion = 1.0;
	vec3  albedo = texture(diffuseMap, vTexcoord).rgb;

	float metal = 0.0f;
	if(hasMetallicTex)
	{
		metal = texture(specularMap, vTexcoord).r;
	}
	else
	{
		metal = metallic;
	}
	
	float roughnessVal;
	if(hasRoughnessTex)
	{
		roughnessVal = texture(shinessMap, vTexcoord).r;
	}
	else
	{
		roughnessVal = roughness;
	}

	int lightCount = lightBuffer.lights[0].lightCount;
	//pbr
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metal);
	for(int i = 0; i < lightCount; i++)
	{
		vec3 lightPos = lightBuffer.lights[i].position.xyz;
		vec3 l = lightPos - fragmentPos;
		float d = length(l);
		l = normalize(l);
		vec3 h = normalize(l + v);
		vec3 lightColor = lightBuffer.lights[i].color.rgb;
		float constant = lightBuffer.lights[i].attentionFactor.r;
		float linear = lightBuffer.lights[i].attentionFactor.g;
		float quadratic = lightBuffer.lights[i].attentionFactor.b;
		vec3 ambient = 0.03 * lightColor * albedo;			
		float attenuation = 1.0 / (constant + linear * d +quadratic * (d * d));
		//check if spotLight
		float intensity = 1.0;
		if(lightBuffer.lights[i].type == 1)
		{
			vec3  lightDirection = normalize(-lightBuffer.lights[i].direction.xyz);
			float costheta = dot(l, lightDirection);
			float epsilon = lightBuffer.lights[i].innerCutoff - lightBuffer.lights[i].outterCutoff;
			intensity = clamp((costheta - lightBuffer.lights[i].outterCutoff), 0.0, 1.0) / epsilon;
		}
		
		float NDF = DistributionGGX(n, h, roughnessVal);   
		float G   = GeometrySmith(n, v, l, roughnessVal);      
		vec3  F   = fresnelSchlick(max(dot(h, v), 0.0), F0);
		   
		vec3 numerator    = NDF * G * F; 
		float denominator = 4.0 * max(dot(n, v), 0.0) * max(dot(n, l), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
		vec3 specular = numerator / denominator;
	
		vec3 Ks = F;
		vec3 Kd = vec3(1.0) - Ks;
		Kd *= (1.0 - metal);
		float NdotL = max(dot(n, l), 0.0);
	
		vec3 ao = vec3(0.03) * lightColor * albedo * ambientOcclusion;
		FragColor.rgb += (Kd * albedo / PI + specular) * NdotL * lightColor *intensity * attenuation;
		FragColor.rgb += ao;
		FragColor.a = 1.0;
		
	}	
	//calc shadows
	if(enableCSM)
	{
		int layer;
		float shadow = calcShadow(fragmentPos, n, layer);
		if(displayCacadedColor)
		{
			if(layer == 0)
			{
				FragColor.rgb *= vec3(1, 0, 0);
			}
			else if(layer == 1)
			{
				FragColor.rgb *= vec3(0, 1, 0);
			}
			else if(layer == 2)
			{
				FragColor.rgb *= vec3(0, 0, 1);
			}
			else if(layer == 3)
			{
				FragColor.rgb *= vec3(1, 0, 1);
			}
		}		
		FragColor.rgb *= vec3((1.0 - shadow));
	}	
}