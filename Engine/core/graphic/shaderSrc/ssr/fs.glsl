#version 460 core

#define MAX_STEP 300
#define thickness 0.1
#define opacity 1
in vec2 vTexcoord;
out vec4 fragColor;

uniform sampler2D  textureColor;
uniform sampler2D  texturePosInViewSpace;
uniform sampler2D  textureNormalInViewSpace;
uniform sampler2D  textureIsMirror;
uniform sampler2D  textureDepth;

layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 viewProjectionMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec4 position;
	vec4 clipRange;
} cameraBuffer;

bool rayOutofScreen(vec2 ray)
{
    return (ray.x > 1 || ray.y > 1 || ray.x < 0 || ray.y < 0) ? true : false;
}

struct Result
{
    bool isHit;
    vec4 color;
};

Result rayMarching3(vec3 rayPos, vec3 dir, int steps)
{
    Result result;
    result.isHit = false;
    result.color = vec4(0.0);
    
    float sampleDepth;
    for(int i = 0; i < steps; ++i)
    {
        rayPos += dir;
        
        if(rayOutofScreen(rayPos.xy))
        {
            break;
        }
        
        sampleDepth = texture(textureDepth, rayPos.xy).r;
        float depthDiff = rayPos.z - sampleDepth;
        if(depthDiff >= 0 && depthDiff < 0.00001f)
        {
            result.isHit = true;
            result.color = texture(textureColor, rayPos.xy);
            break;
        }
    }
    return result;
}


float pointToLineDistance(vec3 x0, vec3 x1, vec3 x2) 
{
    //x0: point, x1: linePointA, x2: linePointB
    //https://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
    return length(cross(x0-x1,x0-x2))/length(x2-x1);
}

float pointPlaneDistance(vec3 point,vec3 planePoint,vec3 planeNormal)
{
	//// https://mathworld.wolfram.com/Point-PlaneDistance.html
	//// https://en.wikipedia.org/wiki/Plane_(geometry)
	//// http://paulbourke.net/geometry/pointlineplane/
	float a=planeNormal.x,b=planeNormal.y,c=planeNormal.z;
	float x0=point.x,y0=point.y,z0=point.z;
	float x=planePoint.x,y=planePoint.y,z=planePoint.z;
	float d=-(a*x+b*y+c*z);
	float distance=a*x0+b*y0+c*z0+d;
	return distance;
}

float getDepth(const in vec2 uv) 
{
	return texture(textureDepth, uv).r;
}

float perspectiveDepthToViewZ( const in float depth, const in float near, const in float far ) 
{
    return ( near * far ) / (far - ( far - near ) * depth );
}

float getViewZ(const in float depth) 
{
    return perspectiveDepthToViewZ(depth, cameraBuffer.clipRange[0], cameraBuffer.clipRange[1]);	
}

vec3 getViewPosition( const in vec2 uv, const in float depth, const in float clipW ) 
{
	vec4 clipPosition = vec4((vec3(uv, depth) - 0.5) * 2.0, 1.0);//ndc
	clipPosition *= 1.0; //clip
	mat4 inverseProjMat = inverse(cameraBuffer.projectionMatrix);
	vec4 viewPos = (inverseProjMat * clipPosition);
	viewPos /= viewPos.w;
	return viewPos.xyz;
}

vec3 getViewNormal( const in vec2 uv ) 
{
	return texture(textureNormalInViewSpace, uv).xyz;
}

vec2 viewPositionToXY(vec3 viewPosition, vec2 screenSize)
{	
	vec4 clip = cameraBuffer.projectionMatrix * vec4(viewPosition,1);
	vec2 xy = clip.xy;
	xy /= clip.w;
	xy = (xy+1.0f)/2.0f;
	xy *= screenSize;
	return xy;
}

void main()
{
	float maxDistance = 0.5f;
    vec2 screenSize  = textureSize(textureColor, 0);    
    int isMirror	  = int(texture(textureIsMirror, vTexcoord).r);    
    fragColor = texture(textureColor, vTexcoord);
    if(isMirror != 0)
    {        
        float depth = getDepth(vTexcoord);
		float viewZ = getViewZ(depth);

		//³¬¹ýÔ¶Æ½Ãæ
		if(viewZ >= cameraBuffer.clipRange[1]) 
		{
			return;
		}

		//float clipW = cameraProjectionMatrix[2][3] * viewZ+cameraProjectionMatrix[3][3];
		//vec3 viewPosition=getViewPosition( vUv, depth, clipW );
		vec3 viewPosition = texture(texturePosInViewSpace, vTexcoord).xyz;
		vec2 d0 = vTexcoord * screenSize;
		vec2 d1;

		vec3 viewNormal = getViewNormal(vTexcoord);
		
		vec3 viewIncidentDir = normalize(viewPosition);
		vec3 viewReflectDir = reflect(viewIncidentDir,normalize(viewNormal));
		

		float maxReflectRayLength = maxDistance/dot(-viewIncidentDir, viewNormal);
		// dot(a,b)==length(a)*length(b)*cos(theta) // https://www.mathsisfun.com/algebra/vectors-dot-product.html
		// if(a.isNormalized&&b.isNormalized) dot(a,b)==cos(theta)
		// maxDistance/maxReflectRayLen=cos(theta)
		// maxDistance/maxReflectRayLen==dot(a,b)
		// maxReflectRayLen==maxDistance/dot(a,b)

		vec3 d1viewPosition = viewPosition + viewReflectDir * maxReflectRayLength;
		//cameraNear
		if(d1viewPosition.z > -cameraBuffer.clipRange[0])
		{
			//https://tutorial.math.lamar.edu/Classes/CalcIII/EqnsOfLines.aspx
			float t = (-cameraBuffer.clipRange[0] - viewPosition.z) / viewReflectDir.z;
			d1viewPosition = viewPosition + viewReflectDir * t;
		}
		
		d1 = viewPositionToXY(d1viewPosition, screenSize);

		float xLen = d1.x-d0.x;
		float yLen = d1.y-d0.y;
		float totalStep = max(abs(xLen),abs(yLen));
		float xSpan = xLen/totalStep;
		float ySpan = yLen/totalStep;
		float sStep = 1.0/totalStep;
		float s = sStep; // start at sStep since loop starts at i=1
	
		for(float i = 1.0; i < float(MAX_STEP); ++i)
		{
			if(i >= totalStep) 
			{
				break;
			}	
			vec2 xy = vec2(d0.x + i * xSpan, d0.y + i * ySpan);
			if(xy.x < 0.0 || xy.x > screenSize.x || xy.y < 0.0 || xy.y > screenSize.y)
			{				
				break;
			}
			vec2 uv = xy / float(screenSize);

			float d = getDepth(uv);
			float vZ = getViewZ(d);
			if(vZ >= cameraBuffer.clipRange[1]) 
			{
				continue;
			}
			float cW = cameraBuffer.projectionMatrix[2][3] * vZ+cameraBuffer.projectionMatrix[3][3];
			vec3 vP = getViewPosition(uv, d, cW);

			
			// https://comp.nus.edu.sg/~lowkl/publications/lowk_persp_interp_techrep.pdf
			float recipVPZ = 1./viewPosition.z;
			float viewReflectRayZ = 1./(recipVPZ+s*(1./d1viewPosition.z-recipVPZ));
			

			// if(viewReflectRayZ>vZ) continue; // will cause "npm run make-screenshot webgl_postprocessing_ssr" high probability hang.
			// https://github.com/mrdoob/three.js/pull/21539#issuecomment-821061164
			if(viewReflectRayZ <= vZ)
			{				
				bool hit;
				//#ifdef INFINITE_THICK
				hit = true;
				//#else
				//	float away=pointToLineDistance(vP,viewPosition,d1viewPosition);
				//
				//	float minThickness;
				//	vec2 xyNeighbor=xy;
				//	xyNeighbor.x+=1.;
				//	vec2 uvNeighbor=xyNeighbor/screenSize;
				//	vec3 vPNeighbor=getViewPosition(uvNeighbor,d,cW);
				//	minThickness=vPNeighbor.x-vP.x;
				//	minThickness*=3.;
				//	float tk=max(minThickness,thickness);
				//
				//	hit=away<=tk;
				//#endif

				if(hit)
				{
					vec3 vN = getViewNormal(uv);
					//if(dot(viewReflectDir,vN) >= 0.0)
					//{
					//	fragColor = vec4(vec3(1.0,0.0,0.0),1.0);
					//	break; // treat backfaces as opaque
					//}
					float distance = pointPlaneDistance(vP,viewPosition,viewNormal);
					//if(distance > maxDistance) 
					//{
					//	
					//	break;
					//}
					float op = opacity;
					
					float ratio = 1. - (distance/maxDistance);
					float attenuation = ratio*ratio;
					op = opacity * attenuation;
					
					
					vec4 reflectColor = texture(textureColor, uv);
					fragColor.rgb = reflectColor.rgb;
					fragColor.a = 1.0;

					fragColor = vec4(fragColor.rgb,1.0);
					break;
					break;
				}
			}
			s+=sStep;
		}
    }
}