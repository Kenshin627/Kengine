#version 460 core

layout(location = 0) out vec4 fragColor;

layout(binding = 0) uniform sampler2D accum;
layout(binding = 1) uniform sampler2D reveal;

const float EPSILON = 0.00001f;

bool isApproximatelyEqual(float a, float b)
{
    return abs(a - b) <= (abs(a) < abs(b) ? abs(b) : abs(a)) * EPSILON;
}

float max3(vec3 v)
{
    return max(max(v.x, v.y), v.z);
}

void main()
{
    ivec2 coords = ivec2(gl_FragCoord.xy);

    float revealage = texelFetch(reveal, coords, 0).r;

    if(isApproximatelyEqual(revealage, 1.0f))
    {
        discard;
    }

    vec4 accumulation = texelFetch(accum, coords, 0);
    if(isinf(max3(abs(accumulation.rgb))))
    {
        accumulation.rgb = vec3(accumulation.a);
    }

    vec3 averageColor = accumulation.rgb / max(accumulation.a, EPSILON);

    fragColor = vec4(averageColor, 1.0 - revealage);
}