#version 460 core
#define MAX_LIGHT_COUNT 16
#define PI 3.14159265358979

//TO SCREEN
layout (location = 0) out vec4 FragColor;

in vec2 vTexcoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gSpecShiness;
uniform sampler2D gMaterialType;
uniform sampler2D ssaoMap;
uniform int lightCount;

layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 viewProjectionMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec3 position;
} cameraBuffer;

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

layout (std140, binding = 1) uniform LightBuffer
{
	Light lights[MAX_LIGHT_COUNT];
} lightBuffer;

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

void main()
{
	//calc illumulation in vewSpace
	vec3  fragmentPos	   =  texture(gPosition, vTexcoord).xyz;
	vec3  n				   =  normalize(texture(gNormal, vTexcoord).xyz);
	vec3  v				   = normalize(-fragmentPos);
	float ambientOcclusion = texture(ssaoMap, vTexcoord).r;
	float materialType     = texture(gMaterialType, vTexcoord).r;

	vec4  diffEmissive	   = texture(gDiffuse, vTexcoord);
	vec3  diff			   = pow(diffEmissive.rgb, vec3(2.2));              //for pbr this is albedo
	float emmisive		   = diffEmissive.a;
	//check if si emissiveLighting return it's emissiveColor, no calc lghting
	if(emmisive == 1.0)
	{
		FragColor = vec4(diff, 1.0);
	}
	else
	{
		vec4 specShiness	   = texture(gSpecShiness, vTexcoord);
		float spec			   = specShiness.r;       //for pbr this is metallic
		float shiness		   = specShiness.a;       //for pbr this is roughness
		float metallic		   = specShiness.r;
		float roughness		   = specShiness.g;
		int   lightCount	   = lightBuffer.lights[0].lightCount;
		//pbr
		vec3 F0 = vec3(0.04);
		F0 = mix(F0, diff, metallic);
		for(int i = 0; i < lightCount; i++)
		{
			vec4 viewLightPos = cameraBuffer.viewMatrix * vec4(lightBuffer.lights[i].position.xyz, 1.0);
			vec3 lightPos = vec3(viewLightPos);
			vec3 l = lightPos - fragmentPos;
			float d = length(l);
			l = normalize(l);
			vec3 h = normalize(l + v);
			vec3 lightColor = lightBuffer.lights[i].color.rgb;
			float constant = lightBuffer.lights[i].attentionFactor.r;
			float linear = lightBuffer.lights[i].attentionFactor.g;
			float quadratic = lightBuffer.lights[i].attentionFactor.b;
			vec3 ambient = 0.03 * lightColor * diff;			
			float attenuation = 1.0 / (constant + linear * d +quadratic * (d * d));
			//check if spotLight
			float intensity = 1.0;
			if(lightBuffer.lights[i].type == 1)
			{
				vec3  lightDirection = normalize(vec3(cameraBuffer.viewMatrix * -lightBuffer.lights[i].direction));
				float costheta = dot(l, lightDirection);
				float epsilon = lightBuffer.lights[i].innerCutoff - lightBuffer.lights[i].outterCutoff;
				intensity = clamp((costheta - lightBuffer.lights[i].outterCutoff), 0.0, 1.0) / epsilon;
			}

			if(materialType == 0.0)
			{
				vec3 diffuse = max(dot(n, l), 0.0) * lightColor * diff;
				vec3 specular = pow(max(dot(n, h), 0.0), shiness) * lightColor * spec;
				diffuse   *= attenuation;
				diffuse   *= intensity;
				specular  *= attenuation;
				specular  *= intensity;
				ambient   *= ambientOcclusion;
				FragColor += vec4(specular + ambient + diffuse, 1.0);
			}
			else if(materialType == 1.0)
			{
				float NDF = DistributionGGX(n, h, roughness);   
				float G   = GeometrySmith(n, v, l, roughness);      
				vec3  F   = fresnelSchlick(max(dot(h, v), 0.0), F0);
				   
				vec3 numerator    = NDF * G * F; 
				float denominator = 4.0 * max(dot(n, v), 0.0) * max(dot(n, l), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
				vec3 specular = numerator / denominator;

				vec3 Ks = F;
				vec3 Kd = vec3(1.0) - Ks;
				Kd *= (1.0 - metallic);
				float NdotL = max(dot(n, l), 0.0);

				vec3 ao = vec3(0.03) * lightColor * diff * ambientOcclusion;
				FragColor.rgb += (Kd * diff / PI + specular) * NdotL * lightColor *intensity * attenuation;
				FragColor.rgb += ao;
				FragColor.a = 1.0;
			}
		}	
	}	
}