#version 460 core

#define PI                         3.1415926535897932384626433832795
#define HALF_PI                    1.5707963267948966192313216916398

layout(local_size_x = 8, local_size_y = 8) in;

struct GTAOSettings
{
   float         effectRadius;
   float         radiusMultiplier;
   float         effectFalloffRange;
   float         sampleDistributionPower;
   float         thinOccluderCompensation;
   int           sliceCount;
   int           stepsPerSlice;
   float         depthMIPSamplingOffset;
   int           depthMipLevel;
   float         finalValuePow;
   float         occlusionTermScale; 
   float         padding0;

   vec2          ndcToViewMulXPixelSize;
   vec2          ndcToViewMul;
   vec2          ndcToViewAdd;
   vec2          viewportPixelSize;
   vec2          depthUnpackConsts;
   vec2          padding1;
};

layout(std140, binding = 2) uniform GTAOSettingsBuffer
{
    GTAOSettings gtaoSettings;
} gtaoSettingBuffer;

layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 viewProjectionMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec4 position;
	vec4 clipRange;
} cameraBuffer;

uniform sampler2D     depthTex;  //viewSpace

layout(binding = 0, rgba16f)      uniform readonly image2D viewSpaceNormalTex;
layout(binding = 1, r16ui)          uniform readonly uimage2D hilbertLUT; //64 * 64
layout(binding = 2, rgba16f)      uniform readonly image2D viewSpacePositionTex;
layout(binding = 3, r32f)         uniform writeonly image2D outEdgesTex;
layout(binding = 4, rgba32f)      uniform writeonly image2D outAOTex;

float linearZ(float ndcDepth)
{
    float near = cameraBuffer.clipRange.x;
    float far = cameraBuffer.clipRange.y;
    return (near * far) / (far - (far - near) * ndcDepth);
}

//TODO?
vec3 computeViewSpacePosition(vec2 uv, float viewSpaceZ)
{
    vec3 ret;
    ret.xy = (gtaoSettingBuffer.gtaoSettings.ndcToViewMul * uv + gtaoSettingBuffer.gtaoSettings.ndcToViewAdd) * -viewSpaceZ;
    ret.z = -viewSpaceZ;
    return ret;
}

float linearDepth(float screenDepth)
{
    float depthLinearMul = gtaoSettingBuffer.gtaoSettings.depthUnpackConsts.x;
    float depthLinearAdd = gtaoSettingBuffer.gtaoSettings.depthUnpackConsts.y;
    return depthLinearMul / (depthLinearAdd - screenDepth);
}

vec4 calculateEdges(float centerZ, float leftZ, float rightZ, float topZ, float bottomZ)
{
    vec4 edgesLRTB = vec4(leftZ, rightZ, topZ, bottomZ) - vec4(centerZ);
    float slopeLR = (edgesLRTB.y - edgesLRTB.x) * 0.5;
    float slopeTB = (edgesLRTB.w - edgesLRTB.z) * 0.5;
    vec4 edgesLRTBSlopeAdjusted = edgesLRTB + vec4(slopeLR, -slopeLR, slopeTB, -slopeTB);
    edgesLRTB = min(abs(edgesLRTB), abs(edgesLRTBSlopeAdjusted));
    return vec4(clamp((1.25f - edgesLRTB / (centerZ * 0.011f)), vec4(0.0f), vec4(1.0f)));
}

float packEdge(vec4 edgesLRTB)
{
    edgesLRTB = round(clamp(edgesLRTB, 0.0, 1.0) * 2.9);
    return dot(edgesLRTB, vec4( 64.0 / 255.0, 16.0 / 255.0, 4.0 / 255.0, 1.0 / 255.0));
}

vec2 spatioTemporalNoise(ivec2 pixCoord, uint temporalIndex)
{
    vec2 noise;
    uint index = uint(imageLoad(hilbertLUT, ivec2(pixCoord % 64)).x);
    index += 288*(temporalIndex % 64);
    return vec2(fract(0.5 + index * vec2(0.75487766624669276005, 0.5698402909980532659114)));
}

//TODO
//float fastSqrt(float inX)
//{
//    return (float)(asfloat( 0x1fbd1df5 + ( asint( x ) >> 1 ) ));
//}

float fastACos(float inX)
{
    float x = abs(inX);
    float res = -0.156583 * x + HALF_PI;
    res *= sqrt(1.0 - x);
    return (inX >= 0) ? res : PI - res;
}

void outputWorkingTerm(ivec2 pixCoord, float visibility)
{
    visibility = clamp(visibility / gtaoSettingBuffer.gtaoSettings.occlusionTermScale, 0.0f, 1.0f);
    imageStore(outAOTex, pixCoord, vec4(visibility));
}

vec3 calcViewSpaceNormal(vec4 edgesLRTB, vec3 pixCenterPos, vec3 pixLPos, vec3 pixRPos, vec3 pixTPos, vec3 pixBPos)
{
      // Get this pixel's viewspace normal
    vec4 acceptedNormals  = clamp(vec4(edgesLRTB.x*edgesLRTB.z, edgesLRTB.z*edgesLRTB.y, edgesLRTB.y*edgesLRTB.w, edgesLRTB.w*edgesLRTB.x) + vec4(0.01f), 0.0f, 1.0f);

    pixLPos = normalize(pixLPos - pixCenterPos);
    pixRPos = normalize(pixRPos - pixCenterPos);
    pixTPos = normalize(pixTPos - pixCenterPos);
    pixBPos = normalize(pixBPos - pixCenterPos);

    vec3 pixelNormal =  acceptedNormals.x * cross(pixLPos, pixTPos ) +
                        + acceptedNormals.y * cross(pixTPos, pixRPos ) +
                        + acceptedNormals.z * cross(pixRPos, pixBPos ) +
                        + acceptedNormals.w * cross(pixBPos, pixLPos );
    pixelNormal = normalize( pixelNormal );

    return pixelNormal;
}

void main()
{
    ivec2 pixCoord = ivec2(gl_GlobalInvocationID);
    vec2 normalizedScreenPos =  (pixCoord + vec2(0.5)) * gtaoSettingBuffer.gtaoSettings.viewportPixelSize; //NORMALIZE TO [0, 1]

    //      | x | y |
    // | x | w | z |
    // | w | z |
    //vec4 TR = textureGather(depthTex, vec2(pixCoord), 0);
    //vec4 BL = textureGatherOffset(depthTex, vec2(pixCoord), ivec2(-1, -1), 0);
    //float viewSpaceZ = TR.w;
    //float pixLZ      = BL.x;
    //float pixTZ      = TR.x;
    //float pixRZ      = TR.z;
    //float pixBZ      = BL.z;
    vec2 dx = vec2(gtaoSettingBuffer.gtaoSettings.viewportPixelSize.x, 0);
    vec2 dy = vec2(0, gtaoSettingBuffer.gtaoSettings.viewportPixelSize.y);

    float viewSpaceZ = textureLod(depthTex, normalizedScreenPos, 0).r;
    float pixLZ      = textureLod(depthTex, normalizedScreenPos - dx, 0).r;
    float pixTZ      = textureLod(depthTex, normalizedScreenPos + dy, 0).r;
    float pixRZ      = textureLod(depthTex, normalizedScreenPos + dx, 0).r;
    float pixBZ      = textureLod(depthTex, normalizedScreenPos - dy, 0).r;
    //viewSpaceZ *= 0.99999; 

    vec4 edgesLRTB = calculateEdges(viewSpaceZ, pixLZ, pixRZ, pixTZ, pixBZ);
    float packedEdgesData = packEdge(edgesLRTB);
    imageStore(outEdgesTex, pixCoord, vec4(packedEdgesData));

    vec3 viewSpacePosition = /** imageLoad(viewSpacePositionTex, pixCoord).xyz;**/computeViewSpacePosition(normalizedScreenPos, viewSpaceZ );
    vec3 LEFT       = computeViewSpacePosition(normalizedScreenPos + vec2(-1,  0) * gtaoSettingBuffer.gtaoSettings.viewportPixelSize, pixLZ);
    vec3 RIGHT      = computeViewSpacePosition(normalizedScreenPos + vec2( 1,  0) * gtaoSettingBuffer.gtaoSettings.viewportPixelSize, pixRZ);
    vec3 TOP        = computeViewSpacePosition(normalizedScreenPos + vec2( 0,  1) * gtaoSettingBuffer.gtaoSettings.viewportPixelSize, pixTZ);
    vec3 BOTTOM     = computeViewSpacePosition(normalizedScreenPos + vec2( 0, -1) * gtaoSettingBuffer.gtaoSettings.viewportPixelSize, pixBZ);
    vec3 viewSpaceNormal = calcViewSpaceNormal(edgesLRTB, viewSpacePosition, LEFT, RIGHT, TOP, BOTTOM);
    //vec3 viewSpaceNormal = imageLoad(viewSpaceNormalTex, pixCoord).xyz;
    viewSpaceNormal = normalize(-viewSpaceNormal);

    
    //vec3 viewSpacePosition = computeViewSpacePosition(normalizedScreenPos, viewSpaceZ);//imageLoad(viewSpacePositionTex, pixCoord).xyz;
    vec3 viewVec = normalize(-viewSpacePosition);

    float effRadius = gtaoSettingBuffer.gtaoSettings.effectRadius * gtaoSettingBuffer.gtaoSettings.radiusMultiplier;
    float falloffRange = gtaoSettingBuffer.gtaoSettings.effectFalloffRange * effRadius;
    float falloffFrom = effRadius * (1.0f - gtaoSettingBuffer.gtaoSettings.effectFalloffRange);
    float falloffMul = -1.0f / falloffRange;
    float falloffAdd = falloffFrom/falloffRange + 1.0f;

    float visibility = 0.0f;

    //TODO
    vec3 bentNormal = viewSpaceNormal;

    vec2 localNoise = spatioTemporalNoise(pixCoord, 0);
    float noiseSlice = localNoise.x;
    float noiseSample = localNoise.y;

    float pixelTooCloseThreshold = 1.3;
    
    //TODO:?
    vec2 pixelDirRBViewspaceSizeAtCenterZ = viewSpaceZ.xx * gtaoSettingBuffer.gtaoSettings.ndcToViewMulXPixelSize;

    float screenSpaceRadius = effRadius / float(pixelDirRBViewspaceSizeAtCenterZ.x);

    visibility += clamp((10 - screenSpaceRadius)/100, 0.0f, 1.0f) * 0.5;

    float minS = pixelTooCloseThreshold / screenSpaceRadius;

    //slice
    //#if 0
    for(float slice = 0; slice < gtaoSettingBuffer.gtaoSettings.sliceCount; ++slice)
    {
        float sliceK = (slice + noiseSlice) / gtaoSettingBuffer.gtaoSettings.sliceCount;
        float phi = sliceK * PI;
        float cosPhi = cos(phi);
        float sinPhi = sin(phi);
        //TODO?
        vec2 omega = vec2(cosPhi, -sinPhi);
        omega *= screenSpaceRadius;
    
        vec3 directionVec = vec3(cosPhi, sinPhi, 0);
        vec3 orthoDirectionVec= directionVec - (dot(viewVec, directionVec) * viewVec);
        vec3 axisVec = normalize(cross(orthoDirectionVec, viewVec));
    
        vec3 projectedNormalVec = viewSpaceNormal - axisVec *dot(viewSpaceNormal, axisVec);
    
        float signNorm = sign(dot(orthoDirectionVec, projectedNormalVec));
        float projectedNormalVecLength = length(projectedNormalVec);
    
        float cosNorm = float(clamp(dot(projectedNormalVec, viewVec) / projectedNormalVecLength, 0.0f, 1.0f));
        float n = signNorm * acos(cosNorm);
    
        float lowHorizonCos0 = cos(n + HALF_PI);
        float lowHirizonCos1 = cos(n - HALF_PI);
    
        float horizonCos0 = lowHorizonCos0;
        float horizonCos1 = lowHirizonCos1;
    
        for(float step = 0; step < gtaoSettingBuffer.gtaoSettings.stepsPerSlice; ++step)
        {
            float stepBaseNoise = float(slice + step * gtaoSettingBuffer.gtaoSettings.stepsPerSlice) * 0.6180339887498948482; // <- this should unroll
            float stepNoise = fract(noiseSample + stepBaseNoise);
            float s = (step + stepNoise) / float(gtaoSettingBuffer.gtaoSettings.stepsPerSlice);
            s = float(pow(s, gtaoSettingBuffer.gtaoSettings.sampleDistributionPower));
            s += minS;
    
            vec2 sampleOffset = s * omega;
            float sampleOffsetLength = length(sampleOffset);
    
            float mipLevel = clamp(log2(sampleOffsetLength) - gtaoSettingBuffer.gtaoSettings.depthMIPSamplingOffset, 0, gtaoSettingBuffer.gtaoSettings.depthMipLevel - 1);
    
            sampleOffset = round(sampleOffset) * gtaoSettingBuffer.gtaoSettings.viewportPixelSize;
    
            vec2 sampleScreenPos0 = normalizedScreenPos + sampleOffset;
            float sz0 = textureLod(depthTex, sampleScreenPos0, mipLevel).r;
            vec3 samplePos0 = computeViewSpacePosition(sampleScreenPos0, sz0);
    
            vec2 sampleScreenPos1 = normalizedScreenPos - sampleOffset;
            float sz1 = textureLod(depthTex, sampleScreenPos1, mipLevel).r;
            vec3 samplePos1 = computeViewSpacePosition(sampleScreenPos1, sz1);
    
            vec3 sampleDelta0 = samplePos0 - viewSpacePosition;
            vec3 sampleDelta1 = samplePos1 - viewSpacePosition;
    
            float sampleDist0 = length(sampleDelta0);
            float sampleDist1 = length(sampleDelta1);
    
            vec3 sampleHorizonVec0 = vec3(sampleDelta0 / sampleDist0);
            vec3 sampleHorizonVec1 = vec3(sampleDelta1 / sampleDist1);
    
            //float weight0 = clamp(sampleDist0 * falloffMul + falloffAdd, 0.0f, 1.0f);
            //float weight1 = clamp(sampleDist1 * falloffMul + falloffAdd, 0.0f, 1.0f);
    
            float falloffBase0 = length(vec3(sampleDelta0.x, sampleDelta0.y, sampleDelta0.z * (1 + gtaoSettingBuffer.gtaoSettings.thinOccluderCompensation)));
            float falloffBase1 = length(vec3(sampleDelta1.x, sampleDelta1.y, sampleDelta1.z * (1 + gtaoSettingBuffer.gtaoSettings.thinOccluderCompensation)));
            float weight0 = clamp(falloffBase0 * falloffMul + falloffAdd, 0.0f, 1.0f);
            float weight1 = clamp(falloffBase1 * falloffMul + falloffAdd, 0.0f, 1.0f);
            //weight0 = weight1 = 1.0;
            float shc0 = dot(sampleHorizonVec0, viewVec);
            float shc1 = dot(sampleHorizonVec1, viewVec);
    
            shc0 = mix(lowHorizonCos0, shc0, weight0);
            shc1 = mix(lowHirizonCos1, shc1, weight1);
    
            horizonCos0 = max(horizonCos0, shc0);
            horizonCos1 = max(horizonCos1, shc1);
        }
        projectedNormalVecLength = mix(projectedNormalVecLength, 1.0f, 0.05f);
    
        //TODO
        float h0 =  -acos(horizonCos1);
        float h1 =   acos(horizonCos0);
    
        float iarc0 = (cosNorm + 2.0f * h0 * sin(n)-cos(2.0f * h0 - n))/4.0f;
        float iarc1 = (cosNorm + 2.0f * h1 * sin(n)-cos(2.0f * h1 - n))/4.0f;
        float localVisibility = projectedNormalVecLength * (iarc0 + iarc1);
        visibility += localVisibility;
    }
    visibility /= float(gtaoSettingBuffer.gtaoSettings.sliceCount);
    visibility = pow(visibility, gtaoSettingBuffer.gtaoSettings.finalValuePow);
    visibility = max(0.03f, visibility);
    
    
    //imageStore(outAOTex, pixCoord, vec4(vec3(localNoise.xy, 0.0), 1.0));
    //#endif
    outputWorkingTerm(pixCoord, visibility);
}