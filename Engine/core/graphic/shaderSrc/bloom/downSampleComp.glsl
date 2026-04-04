#version 460 core
layout(local_size_x = 8, local_size_y = 8) in;

#define PI 3.14159265358
#define E  2.71828172846

//layout(binding = 0, rgba16f) uniform readonly image2D  uSourceTex;
layout(binding = 1, rgba16f) uniform writeonly image2D uDestTex;

uniform sampler2D uSourceTex;
uniform int       uDownSampleBlurSize;
uniform float     uDownSampleBlurSigma;
uniform bool      uFirstDownSample;
uniform float     uBloomIntensity;
uniform int       uSourceMipmap;

float GaussWeight2D(float x, float y, float sigma)
{    
    float sigma2 = pow(sigma, 2);
    float a = -(x*x + y*y) / (2.0 * sigma2);
    return pow(E, a) / (2.0 * PI * sigma2);
}

vec3 GaussNxN(vec2 uv)
{
    vec3 color = vec3(0, 0, 0);
    //ivec2 sourceCenter = uv * 2;
    int r = uDownSampleBlurSize / 2;
    vec2 sourceSize = textureSize(uSourceTex, uSourceMipmap);
    vec2 sourceTexelSize = 1.0 / sourceSize;
    float weight = 0.0;

    for(int i=-r; i<=r; ++i)
    {
        for(int j=-r; j<=r; ++j)
        {
            vec2 sourceCoord = uv + vec2(i, j) * sourceTexelSize;
            vec3 c = textureLod(uSourceTex, sourceCoord, uSourceMipmap).rgb;
            float luma = dot(vec3(0.2126, 0.7152, 0.0722), c);

            float w1 = GaussWeight2D(i, j, uDownSampleBlurSigma);
            float w2 = uFirstDownSample? 1.0 / (1.0 + luma) : 1.0;
            float w = w1 * w2;
            
            color += c * w;
            weight += w;
        }
    }

    color /= weight;
    return color;
}

void main()
{
    ivec2 uv = ivec2(gl_GlobalInvocationID);
    vec2 size = imageSize(uDestTex);
    vec2 sourceUV = (uv + vec2(0.5)) / size;
    vec2 sourceSize = textureSize(uSourceTex, uSourceMipmap);
    if(uv.x <= size.x && uv.y <= size.y)
    {       
        vec3 c = GaussNxN(sourceUV) * uBloomIntensity;
        imageStore(uDestTex, uv, vec4(c, 1.0));
    }
}