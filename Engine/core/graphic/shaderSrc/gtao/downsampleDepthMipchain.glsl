#version 460 core

layout(local_size_x = 8, local_size_y = 8) in;

layout(binding = 0, r32f) uniform writeonly image2D uDstTex;
layout(binding = 1, r32f) uniform readonly image2D uSrcTex;

uniform float     effectRadius;  // 0.0 - 10000.0 0.05f
uniform float     radiusMultiplier;  //0.3 - 3.0   step 0.05
uniform float     effectFalloffRange; // 0.0 - 1.0   step 0.01, 0.05f

void main()
{
    ivec2 dstCoord = ivec2(gl_GlobalInvocationID);
    ivec2 dstSize = imageSize(uDstTex);

    if(dstCoord.x >= dstSize.x || dstCoord.y >= dstSize.y)
    {
        return;
    }

    ivec2 srcBase = dstCoord * 2;
    ivec2 srcSize = imageSize(uSrcTex);
    float d[4];
    ivec2 coord;

    d[0] = imageLoad(uSrcTex,ivec2(srcBase.x, srcBase.y)).r;

    d[1] = imageLoad(uSrcTex, ivec2(srcBase.x + 1, srcBase.y)).r;

    d[2] = imageLoad(uSrcTex, ivec2(srcBase.x, srcBase.y + 1)).r;

    d[3] = imageLoad(uSrcTex, ivec2(srcBase.x + 1, srcBase.y + 1)).r;

    float maxDepth = max(max(d[0], d[1]), max(d[2], d[3]));

    const float depthRangeScaleFactor = 0.75;

    float effectRadius = depthRangeScaleFactor * effectRadius * radiusMultiplier;
    float falloffRange = effectFalloffRange * effectRadius;
    float falloffFrom = effectRadius * (1.0f - effectFalloffRange);
    float falloffMul = -1.0f / falloffRange;
    float falloffAdd = falloffFrom / (falloffRange) + 1.0f;

    float weight[4];
    weight[0] = clamp((maxDepth - d[0]) * falloffMul + falloffAdd, 0.0f, 1.0f);
    weight[1] = clamp((maxDepth - d[1]) * falloffMul + falloffAdd, 0.0f, 1.0f);
    weight[2] = clamp((maxDepth - d[2]) * falloffMul + falloffAdd, 0.0f, 1.0f);
    weight[3] = clamp((maxDepth - d[3]) * falloffMul + falloffAdd, 0.0f, 1.0f);

    float weightSum = weight[0] + weight[1] + weight[2] + weight[3];
    float weightedDepth = (d[0] * weight[0] + d[1] * weight[1] + d[2] * weight[2] + d[3] * weight[3])/weightSum;

    imageStore(uDstTex, dstCoord, vec4(weightedDepth));
}