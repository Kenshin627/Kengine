#version 460 core

#define MIN_STEPS 32.0
#define MAX_STEPS 64.0
#define MAX_LIGHT_COUNT 16
#define PI 3.14159265368979

layout (std140, binding = 0) uniform CameraBuffer
{
    mat4 viewProjectionMatrix;
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec4 position;
    vec4 clipRange;
} cameraBuffer;

struct Light
{
    vec4  position;
    vec4  direction;
    vec4  color;
    vec4  attentionFactor;
    float outerCutoff;
    float innerCutoff;
    int   type;
    int   lightCount;
};

layout (std140, binding = 1) uniform LightBuffer
{
    Light lights[MAX_LIGHT_COUNT];
} lightBuffer;

uniform mat4 modelMatrix;
uniform mat4 modelMatrixInvertTranspose;

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexcoord;
in mat3 vTBN;

//only has value in heightMap
in vec3 tangentSpaceViewPos;
in vec3 tangentSpaceFragPos;

uniform sampler2D diffuseMap;      //diffuse or albedo
uniform sampler2D specularMap;     //spec or metallic
uniform sampler2D normalMap;       //normal
uniform sampler2D shinessMap;    //shininess or roughness
uniform sampler2D heightMap;       //displace mapping
uniform int       materialType;
uniform bool      hasHeightTex;
uniform float     pomScale;
uniform bool      enablePOM;

////////////////////BLINN PHONG/////////////////////////////
uniform vec3      diffuseColor;
uniform vec3      specularColor;
uniform vec3      emissiveColor;
uniform float     shiness;
uniform bool      hasDiffTex;
uniform bool      hasSpecTex;
uniform bool      hasNormalTex;
uniform bool      hasShinessTex;
uniform bool      isEmissive;
///////////////////////////////////////////////////////////

////////////////////PBR////////////////////////////////////
uniform vec3      albedoColor;
uniform float     metallic;
uniform float     roughness;
uniform bool      hasAlbedoTex;
uniform bool      hasMetallicTex;
uniform bool      hasRoughnessTex;
///////////////////////////////////////////////////////////

layout(location = 0) out vec4 accum;
layout(location = 1) out float reveal;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
        float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom  = a2;
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

vec2 ParallaxOcclusionMap(vec2 texCoord, vec3 tangentSpaceViewDir)
{
    float numLayers         = mix(MIN_STEPS, MAX_STEPS, max(dot(vec3(0,0,1), tangentSpaceViewDir), 0.0));
    float deltaDepth        = 1.0 / numLayers;
    float currentLayerDepth = 0.0;
    vec2  currentTexcoord   = texCoord;
    //load heightmap need covert to displace map
    float currentMapDepth   = 1.0 - texture(heightMap, currentTexcoord).r;
    vec2 deltaTexcoord = tangentSpaceViewDir.xy / tangentSpaceViewDir.z * pomScale;
    deltaTexcoord /= numLayers;

    while(currentLayerDepth < currentMapDepth)
    {
        currentLayerDepth += deltaDepth;
        currentTexcoord   -= deltaTexcoord;
        currentMapDepth   = 1.0 - texture(heightMap, currentTexcoord).r;
    }
    vec2 lastStepTexcoord = currentTexcoord + deltaTexcoord;
    float currentDepthRatio = currentMapDepth - currentLayerDepth;
    float lastDepthRatio = 1.0 - texture(heightMap, lastStepTexcoord).r - currentLayerDepth + deltaDepth;
    float weight = currentDepthRatio / (currentDepthRatio - lastDepthRatio);
    return (1.0 - weight) * currentTexcoord + weight * lastStepTexcoord;
}

void main()
{
    vec2 texCoord = vTexcoord;
    vec3 n;
    if(hasHeightTex && enablePOM)
    {
        vec3 tangentSpaceViewDir = normalize(tangentSpaceViewPos - tangentSpaceFragPos);
        texCoord = ParallaxOcclusionMap(vTexcoord, tangentSpaceViewDir);
    }

    if(hasNormalTex)
    {
        n = texture(normalMap, texCoord).xyz;
        n = n * 2.0 - 1.0;
        n = normalize(vTBN * n);
    }
    else
    {
        n = normalize(vNormal);
    }

    //blinnphong
    vec4 diffuseEmmisive;
    vec4 specShiness;
    if(materialType == 0)
    {
        if(isEmissive)
        {
            diffuseEmmisive.rgb = emissiveColor;
            diffuseEmmisive.a = 1.0;
        }
        else if(hasDiffTex)
        {
            diffuseEmmisive.rgb = texture(diffuseMap, texCoord).rgb;
            diffuseEmmisive.a = 0.0;
        }
        else
        {
            diffuseEmmisive.rgb = diffuseColor;
            diffuseEmmisive.a = 0.0;
        }

        if(hasSpecTex)
        {
            specShiness.rgb = texture(specularMap, texCoord).rgb;
        }
        else
        {
            specShiness.rgb = specularColor;
        }

        if(hasShinessTex)
        {
            specShiness.a = texture(shinessMap, texCoord).r;
        }
                else
        {
            specShiness.a = shiness;
        }
    }
    //pbr
    else if(materialType == 1)
    {
        if(hasAlbedoTex)
        {
            diffuseEmmisive.rgb = texture(diffuseMap, texCoord).rgb;
        }
        else
        {
            diffuseEmmisive.rgb = albedoColor;
        }

        if(hasMetallicTex)
        {
            specShiness.r = texture(specularMap, texCoord).r;
        }
        else
        {
            specShiness.r = metallic;
        }

        if(hasRoughnessTex)
        {
            specShiness.g = texture(shinessMap, texCoord).r;
        }
        else
        {
            specShiness.g = roughness;
        }
    }

    //calc illumination in viewSpace
    vec3 fragmentPos  = vPos;
    vec3 v            = normalize(-fragmentPos);
    
    float ambientOcclusion = 1.0;
    vec4 fragColor;
    vec3 diff         = diffuseEmmisive.rgb;
    float emmisive    = diffuseEmmisive.a;
    //check if si emissivelighting return it's emissiveColor, no calc lighting
    if (emmisive == 1.0)
    {
        fragColor = vec4(diff, 1.0);
    }
    else
    {
        //calculate lighting
        float spec    = specShiness.r;        //for pbr this is metallic
        float shiness = specShiness.a;        //for pbr this is roughness
        float metallic = specShiness.r;
        float roughness = specShiness.g;
        int  lightCount = lightBuffer.lights[0].lightCount;
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
            float attenuation = 1.0 / (constant + linear * d + quadratic * (d * d));
            //check if spotlight
            float intensity = 1.0;
            if(lightBuffer.lights[i].type == 1)
            {
                vec3 lightDirection = normalize(vec3(cameraBuffer.viewMatrix * -lightBuffer.lights[i].direction));
                float costheta = dot(l, lightDirection);
                float epsilon = lightBuffer.lights[i].innerCutoff - lightBuffer.lights[i].outerCutoff;
                intensity = clamp((costheta - lightBuffer.lights[i].outerCutoff), 0.0, 1.0) / epsilon;
            }

            if(materialType == 0.0)
            {
                vec3 diffuse = max(dot(n, l), 0.0) * lightColor * diff;
                vec3 specular = pow(max(dot(n, h), 0.0), shiness) * lightColor * spec;
                diffuse  *= attenuation;
                diffuse  *= intensity;
                specular *= attenuation;
                specular *= intensity;
                ambient  *= ambientOcclusion;
                fragColor += vec4(specular + ambient + diffuse, 1.0);
            }
            else if(materialType == 1.0)
            {
                float NDF = DistributionGGX(n, h, roughness);
                float G   = GeometrySmith(n, v, l, roughness);
                vec3  F   = fresnelSchlick(max(dot(h, v), 0.0), F0);

                vec3 numerator   = NDF * G * F;
                float denominator = 4.0 * max(dot(n, v), 0.0) * max(dot(n, l), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
                vec3 specular = numerator / denominator;

                vec3 Ks = F;
                vec3 Kd = vec3(1.0) - Ks;
                Kd *= (1.0 - metallic);
                float NdotL = max(dot(n, l), 0.0);

                vec3 ao = vec3(0.03) * lightColor * diff * ambientOcclusion;
                fragColor.rgb += (Kd * diff / PI + specular) * NdotL * lightColor * intensity * attenuation;
                //fragColor.rgb += vec3(max(dot(n, v), 0.0));
                fragColor.rgb += ao;
                fragColor.a = 1.0;
            }
        }
    }

  
    //TODO:
    fragColor.a = 0.2f;
    float weight = clamp(pow(min(1.0, fragColor.a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);
    accum = vec4(fragColor.rgb * fragColor.a, fragColor.a) * weight;
    reveal = fragColor.a;
}