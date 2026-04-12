#version 460 core

layout(local_size_x = 8, local_size_y = 8) in;

uniform sampler2D sourceNDCDepth;
uniform vec2 pixelSize;

layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 viewProjectionMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec4 position;
	vec4 clipRange;
} cameraBuffer;

//uniform float depthLinearizeMul;
//uniform float depthLinearizeAdd;

layout(binding = 0, r32f) uniform writeonly image2D outDepthTex;

float linearZ(float ndcDepth)
{
    float near = cameraBuffer.clipRange.x;
    float far = cameraBuffer.clipRange.y;
    return (near * far) / (far - (far - near) * ndcDepth);
}

void main()
{
	 // MIP 0   
    //const uint2 baseCoord = dispatchThreadID;
    //const uint2 pixCoord = baseCoord * 2;
    //float4 depths4 = sourceNDCDepth.GatherRed( depthSampler, float2( pixCoord * consts.ViewportPixelSize ), int2(1,1) );
    //lpfloat depth0 = XeGTAO_ClampDepth( XeGTAO_ScreenSpaceToViewSpaceDepth( depths4.w, consts ) );
    //lpfloat depth1 = XeGTAO_ClampDepth( XeGTAO_ScreenSpaceToViewSpaceDepth( depths4.z, consts ) );
    //lpfloat depth2 = XeGTAO_ClampDepth( XeGTAO_ScreenSpaceToViewSpaceDepth( depths4.x, consts ) );
    //lpfloat depth3 = XeGTAO_ClampDepth( XeGTAO_ScreenSpaceToViewSpaceDepth( depths4.y, consts ) );
    //outDepth0[ pixCoord + uint2(0, 0) ] = (lpfloat)depth0;
    //outDepth0[ pixCoord + uint2(1, 0) ] = (lpfloat)depth1;
    //outDepth0[ pixCoord + uint2(0, 1) ] = (lpfloat)depth2;
    //outDepth0[ pixCoord + uint2(1, 1) ] = (lpfloat)depth3;
    vec2 imageSize = textureSize(sourceNDCDepth, 0);
    vec2 texelSize = 1.0 / imageSize;
    ivec2 pixCoord = ivec2(gl_GlobalInvocationID);
    if(pixCoord.x >= imageSize.x || pixCoord.y >= imageSize.y)
    {
        return;
    }
    vec2 uv = (vec2(pixCoord) + 0.5) * texelSize;
    float ndcZ = texture(sourceNDCDepth, uv).r;
    float viewZ = linearZ(ndcZ);
    imageStore(outDepthTex, pixCoord, vec4(viewZ));
}