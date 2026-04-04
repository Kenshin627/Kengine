#version 460 core

uniform sampler2D uSourceTex;
uniform int       uDownSampleBlurSize;
uniform float     uDownSampleBlurSigma;
uniform bool      uFirstDownSample;
uniform float     uBloomIntensity;

in vec2  vTexcoord;
out vec4 fragColor;

// calculate gaussian weight
float GaussWeight2D(float x, float y, float sigma)
{
    float PI = 3.14159265358;
    float E = 2.71828172846;
    float sigma2 = pow(sigma, 2);

    float a = -(x*x + y*y) / (2.0 * sigma2);
    return pow(E, a) / (2.0 * PI * sigma2);
}

vec3 GaussNxN(vec2 sourceTexelSize)
{
    vec3 color = vec3(0, 0, 0);
    int r = uDownSampleBlurSize / 2;
    float weight = 0.0;

    for(int i=-r; i<=r; ++i)
    {
        for(int j=-r; j<=r; ++j)
        {
            vec2 coord = vTexcoord + vec2(i, j) * sourceTexelSize;
            vec3 c = texture(uSourceTex, coord).rgb;
            float luma = dot(vec3(0.2126, 0.7152, 0.0722), c);

            float w1 = GaussWeight2D(i, j, uDownSampleBlurSigma);
            float w2 = uFirstDownSample? 1.0 / (1.0 + luma) : 1.0;
            float w = w1 * w2;
            
            color += c * w;
            weight += w;
        }
    }

    color /= weight;
    //return vec3(vTexcoord, 0);
    return color;
}

void main()
{
    fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    vec2 sourceTexelSize = 1.0f / textureSize(uSourceTex, 0);
    fragColor.rgb = GaussNxN(sourceTexelSize) * uBloomIntensity;
}