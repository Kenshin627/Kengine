#version 460 core

#define PI                         3.1415926535897932384626433832795
#define HALF_PI                    1.5707963267948966192313216916398

in vec2 vTexcoord;

out vec4 outAo;

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
uniform sampler2D     viewSpaceNormalTex;
uniform sampler2D     hilbertLUT;
uniform sampler2D     viewSpacePositionTex;

//layout(binding = 0, rgba16f)      uniform readonly image2D viewSpaceNormalTex;
//layout(binding = 1, r16ui)          uniform readonly uimage2D hilbertLUT; //64 * 64
//layout(binding = 2, rgba16f)      uniform readonly image2D viewSpacePositionTex;
//layout(binding = 3, r32f)         uniform writeonly image2D outEdgesTex;
//layout(binding = 4, rgba32f)      uniform writeonly image2D outAOTex;

float linearZ(float ndcDepth)
{
    float near = cameraBuffer.clipRange.x;
    float far = cameraBuffer.clipRange.y;
    return (near * far) / (far - (far - near) * ndcDepth);
}

//TODO?
//vec3 computeViewSpacePosition(vec2 uv, float viewSpaceZ)
//{
//    vec3 ret;
//    ret.xy = (gtaoSettingBuffer.gtaoSettings.ndcToViewMul * uv + gtaoSettingBuffer.gtaoSettings.ndcToViewAdd) * viewSpaceZ;
//    ret.z = -viewSpaceZ;
//    return ret;
//}

//float linearDepth(float screenDepth)
//{
//    float depthLinearMul = gtaoSettingBuffer.gtaoSettings.depthUnpackConsts.x;
//    float depthLinearAdd = gtaoSettingBuffer.gtaoSettings.depthUnpackConsts.y;
//    return depthLinearMul / (depthLinearAdd - screenDepth);
//}
//
//vec4 calculateEdges(float centerZ, float leftZ, float rightZ, float topZ, float bottomZ)
//{
//    vec4 edgesLRTB = vec4(leftZ, rightZ, topZ, bottomZ) - vec4(centerZ);
//    float slopeLR = (edgesLRTB.y - edgesLRTB.x) * 0.5;
//    float slopeTB = (edgesLRTB.w - edgesLRTB.z) * 0.5;
//    vec4 edgesLRTBSlopeAdjusted = edgesLRTB + vec4(slopeLR, -slopeLR, slopeTB, -slopeTB);
//    edgesLRTB = min(abs(edgesLRTB), abs(edgesLRTBSlopeAdjusted));
//    return vec4(clamp((1.25f - edgesLRTB / (centerZ * 0.011f)), vec4(0.0f), vec4(1.0f)));
//}

//float packEdge(vec4 edgesLRTB)
//{
//    edgesLRTB = round(clamp(edgesLRTB, 0.0, 1.0) * 2.9);
//    return dot(edgesLRTB, vec4( 64.0 / 255.0, 16.0 / 255.0, 4.0 / 255.0, 1.0 / 255.0));
//}

vec2 spatioTemporalNoise(ivec2 pixCoord, uint temporalIndex)
{
    vec2 noise;
    uint index = uint(texture(hilbertLUT, ivec2(pixCoord % 64)).x);
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

//void outputWorkingTerm(ivec2 pixCoord, float visibility)
//{
//    visibility = clamp(visibility / gtaoSettingBuffer.gtaoSettings.occlusionTermScale, 0.0f, 1.0f);
//    imageStore(outAOTex, pixCoord, vec4(visibility));
//}

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

float IntegrateArcWithCosWeight(vec2 h, float n)
{
    vec2 arc = -cos(2 * h - n) + cos(n) + 2 * h * sin(n);
    return 0.25 * (arc.x + arc.y);
}

void main()
{
    vec3 normalVs = texture(viewSpaceNormalTex, vTexcoord).xyz;
    vec3 posVs = texture(viewSpacePositionTex, vTexcoord).xyz;
    vec3 viewVec = normalize(-posVs);
    float angle, sliceLength, cosN, n;
    vec2 h, h1h2, h1h2length, falloff, H;
    vec3 sliceDirection, planeNormal, planeTangent, sliceNormal, h1, h2;
    vec2 uvOffset;
    float ao = 0.0f;
    for(int i = 0; i < gtaoSettingBuffer.gtaoSettings.sliceCount; ++i)
    {
        angle = (i * PI) / float(gtaoSettingBuffer.gtaoSettings.sliceCount);
        sliceDirection = vec3(cos(angle), sin(angle), 0.0f);
        planeNormal = normalize(cross(sliceDirection, viewVec));
        planeTangent = cross(viewVec, planeNormal);
        sliceNormal = normalVs - planeNormal * dot(normalVs, planeNormal);
        sliceLength = clamp(length(sliceNormal), 0.0f, 1.0f);
        cosN = clamp(dot(normalize(sliceNormal), viewVec), -1.0f, 1.0f);
        n = -sign(dot(sliceNormal, planeTangent)) * acos(cosN);
        h = vec2(-1);
        for(int j = 1; j < gtaoSettingBuffer.gtaoSettings.stepsPerSlice; ++j)
        {
            uvOffset = sliceDirection.xy * vec2(j) * gtaoSettingBuffer.gtaoSettings.viewportPixelSize;
            h1 = texture(viewSpacePositionTex, vTexcoord + uvOffset).xyz - posVs;
            h2 = texture(viewSpacePositionTex, vTexcoord - uvOffset).xyz - posVs;
            h1h2 = vec2(dot(h1, h1), dot(h2, h2));
            h1h2length = sqrt(h1h2);

            H = vec2(dot(h1, viewVec), dot(h2, viewVec)) * h1h2length;
            falloff = clamp(h1h2 * (2 / pow(gtaoSettingBuffer.gtaoSettings.effectRadius, 2)), 0.0f, 1.0f);
            h.x = (H.x > h.x)? mix(H.x, h.x, falloff.x) : mix(H.x, h.x, gtaoSettingBuffer.gtaoSettings.thinOccluderCompensation);
            h.y = (H.y > h.y)? mix(H.y, h.y, falloff.y) : mix(H.y, h.y, gtaoSettingBuffer.gtaoSettings.thinOccluderCompensation);
        }
        h = acos(clamp(h, -1.0f, 1.0f));
        h.x = n + max(-h.x - n, -PI * 0.5);
        h.y = n + min(h.y - n, PI * 0.5);
        ao += sliceLength * IntegrateArcWithCosWeight(h, n) / gtaoSettingBuffer.gtaoSettings.sliceCount;
    }

    //ao = 1 - ao;
    ao = pow(ao * 1.0, 1.5);
    outAo = vec4(vec3(ao), 1.0);
}