#version 460 core

in vec2 vTexcoord;
out vec4 fragColor;
uniform sampler2D uPrevTex;
uniform sampler2D uCurrentTex;
uniform int       uUpSampleBlurSize;
uniform float       uUpSampleBlurSigma;
uniform float     uBloomIntensity;

float GaussWeight2D(float x, float y, float sigma)
{
    float PI = 3.14159265358;
    float E  = 2.71828182846;
    float sigma2 = pow(sigma, 2);

    float a = -(x*x + y*y) / (2.0 * sigma2);
    return pow(E, a) / (2.0 * PI * sigma2);
}

vec3 GaussNxN(sampler2D tex, vec2 stride)
{
    vec3 color = vec3(0, 0, 0);
    int r = uUpSampleBlurSize / 2;
    float weight = 0.0;
    for(int i=-r; i<=r; i++)
    {
        for(int j=-r; j<=r; j++)
        {
            float w = GaussWeight2D(i, j, uUpSampleBlurSigma);
            vec2 coord = vTexcoord + vec2(i, j) * stride;
            color += texture(tex, coord).rgb * w;
            weight += w;
        }
    }

    color /= weight;
    return color;
}

void main()
{
    fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    vec2 currentTexelSize = 1.0f / textureSize(uCurrentTex, 0);
    vec2 prevTexelSize = 0.5f * currentTexelSize;

    vec3 prevMip = GaussNxN(uPrevTex, prevTexelSize);
    vec3 currentMip = GaussNxN(uCurrentTex, currentTexelSize);

    fragColor.rgb = (prevMip + currentMip) * uBloomIntensity;
}