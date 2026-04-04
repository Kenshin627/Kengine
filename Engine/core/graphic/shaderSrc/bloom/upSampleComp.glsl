#version 460 core
#define PI  3.14159265358
#define E   2.71828182846

layout(local_size_x = 8, local_size_y = 8) in;

//layout(binding = 0, rgba16f) uniform image2D uPrevTex;
//layout(binding = 1, rgba16f) uniform image2D uCurrentTex;


layout(binding = 2, rgba16f) uniform image2D uDestTex;

uniform sampler2D uPrevTex;
uniform sampler2D uCurrentTex;
uniform int       uPrevMipLevel;
uniform int       uCurrentMipLevel;
uniform int       uUpSampleBlurSize;
uniform float     uUpSampleBlurSigma;
uniform float     uBloomIntensity;
uniform bool      uFirstUpSample;

float GaussWeight2D(float x, float y, float sigma)
{
    float sigma2 = pow(sigma, 2);

    float a = -(x*x + y*y) / (2.0 * sigma2);
    return pow(E, a) / (2.0 * PI * sigma2);
}

vec3 GaussNxNPrev(vec2 uv, vec2 texelSize)
{
    vec3 color = vec3(0);
    int r = uUpSampleBlurSize / 2;
    float weight = 0.0;
    for(int i=-r; i<=r; i++)
    {
        for(int j=-r; j<=r; j++)
        {
            float w = GaussWeight2D(i, j, uUpSampleBlurSigma);
            vec2 coord = uv + texelSize * vec2(i, j);
            color += textureLod(uPrevTex, coord, uPrevMipLevel).rgb * w;
            weight += w;
        }
    }

    color /= weight;
    return color;
}

vec3 GaussNxNCurrent(vec2 uv, vec2 texelSize)
{
    vec3 color = vec3(0, 0, 0);
    int r = uUpSampleBlurSize / 2;
    float weight = 0.0;
    for(int i=-r; i<=r; i++)
    {
        for(int j=-r; j<=r; j++)
        {
            float w = GaussWeight2D(i, j, uUpSampleBlurSigma);
            vec2 coord = uv + texelSize * vec2(i, j);           
            color += textureLod(uCurrentTex, coord, uCurrentMipLevel).rgb * w;
            weight += w;
        }
    }
    color /= weight;
    return color;
}

void main()
{
    ivec2 uv = ivec2(gl_GlobalInvocationID);
    ivec2 destSize = imageSize(uDestTex);

    vec2 coord = (vec2(uv) + vec2(0.5)) / vec2(destSize);
    vec2 prevTexelSize = uFirstUpSample? 1.0 / textureSize(uCurrentTex, uPrevMipLevel) : 1.0 / textureSize(uPrevTex, uPrevMipLevel);
    vec2 currentTexelSize = 1.0 / textureSize(uCurrentTex, uCurrentMipLevel);
    
    if(uv.x < destSize.x && uv.y < destSize.y)
    {
        vec3 prevMip = GaussNxNPrev(coord, prevTexelSize);
        vec3 currentMip = GaussNxNCurrent(coord, currentTexelSize);
        imageStore(uDestTex, uv, vec4((prevMip + currentMip) * uBloomIntensity, 1.0));
    }
}