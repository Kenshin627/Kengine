#version 460 core

in vec2			  vTexcoord;
uniform sampler2D screenMap;
uniform vec2	  texelSize;

//x: fixThreshold	    [Range(0.0312f, 0.0833f)
//y: relativeThreshold  [Range(0.063f, 0.333f)]
//z: blendFactor        [Range(0.0f, 1.0f)]
uniform vec4	  fxaaConfig; 
uniform int		  fxaaQuality;   //0: low 1: medium 2:high
out vec4		  FragColor;

struct LumaNeighborhood 
{
	float m, n, e, s, w, ne, se, sw, nw;
	float highest, lowest, range;
};

struct FXAAEdge 
{
	bool isHorizontal;
	float pixelStep;
	float lumaGradient, otherLuma;
};

#if defined(FXAA_QUALITY_LOW)
	#define EXTRA_EDGE_STEPS 3
	#define EDGE_STEP_SIZES 1.5, 2.0, 2.0
	#define LAST_EDGE_STEP_GUESS 8.0
#elif defined(FXAA_QUALITY_MEDIUM)
	#define EXTRA_EDGE_STEPS 8
	#define EDGE_STEP_SIZES 1.5, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 4.0
	#define LAST_EDGE_STEP_GUESS 8.0
#else
	#define EXTRA_EDGE_STEPS 10
	#define EDGE_STEP_SIZES 1.0, 1.0, 1.0, 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0
	#define LAST_EDGE_STEP_GUESS 8.0
#endif

const float edgeStepSizes[EXTRA_EDGE_STEPS] = { EDGE_STEP_SIZES };

vec4 GetSource(vec2 uv)
{
	return texture(screenMap, uv);
}

float GetLuma(vec2 uv, float uOffset = 0.0, float vOffset = 0.0) 
{
	uv += vec2(uOffset, vOffset) * texelSize;
	return GetSource(uv).g;
}

LumaNeighborhood GetLumaNeighborhood(vec2 uv) 
{
	LumaNeighborhood luma;
	luma.m = GetLuma(uv);
	luma.n = GetLuma(uv, 0.0, 1.0);
	luma.e = GetLuma(uv, 1.0, 0.0);
	luma.s = GetLuma(uv, 0.0, -1.0);
	luma.w = GetLuma(uv, -1.0, 0.0);
	luma.ne = GetLuma(uv, 1.0, 1.0);
	luma.se = GetLuma(uv, 1.0, -1.0);
	luma.sw = GetLuma(uv, -1.0, -1.0);
	luma.nw = GetLuma(uv, -1.0, 1.0);

	luma.highest = max(max(max(max(luma.m, luma.n), luma.e), luma.s), luma.w);
	luma.lowest = min(min(min(min(luma.m, luma.n), luma.e), luma.s), luma.w);
	luma.range = luma.highest - luma.lowest;
	return luma;
}

bool IsHorizontalEdge(LumaNeighborhood luma) 
{
	float horizontal =
		2.0 * abs(luma.n + luma.s - 2.0 * luma.m) +
		abs(luma.ne + luma.se - 2.0 * luma.e) +
		abs(luma.nw + luma.sw - 2.0 * luma.w);
	float vertical =
		2.0 * abs(luma.e + luma.w - 2.0 * luma.m) +
		abs(luma.ne + luma.nw - 2.0 * luma.n) +
		abs(luma.se + luma.sw - 2.0 * luma.s);
	return horizontal >= vertical;
}

FXAAEdge GetFXAAEdge(LumaNeighborhood luma) 
{
	FXAAEdge edge;
	edge.isHorizontal = IsHorizontalEdge(luma);
	float lumaP, lumaN;
	if (edge.isHorizontal) {
		edge.pixelStep = texelSize.y;
		lumaP = luma.n;
		lumaN = luma.s;
	}
	else {
		edge.pixelStep = texelSize.x;
		lumaP = luma.e;
		lumaN = luma.w;
	}
	float gradientP = abs(lumaP - luma.m);
	float gradientN = abs(lumaN - luma.m);

	if (gradientP < gradientN) {
		edge.pixelStep = -edge.pixelStep;
		edge.lumaGradient = gradientN;
		edge.otherLuma = lumaN;
	}
	else {
		edge.lumaGradient = gradientP;
		edge.otherLuma = lumaP;
	}
	return edge;
}

bool CanSkipFXAA(LumaNeighborhood luma) 
{
	return luma.range < max(fxaaConfig.x, fxaaConfig.y * luma.highest);
}

float GetSubpixelBlendFactor(LumaNeighborhood luma)
{
	float f = 2.0 * (luma.n + luma.e + luma.s + luma.w);
	f += luma.ne + luma.nw + luma.se + luma.sw;
	f *= 1.0 / 12.0;
	f = abs(f - luma.m);
	f = clamp(f / luma.range, 0.0, 1.0);
	f = smoothstep(0, 1, f);
	return f * f * fxaaConfig.z;
}

float GetEdgeBlendFactor(LumaNeighborhood luma, FXAAEdge edge, vec2 uv) 
{
	vec2 edgeUV = uv;
	vec2 uvStep = vec2(0.0);
	if (edge.isHorizontal) {
		edgeUV.y += 0.5 * edge.pixelStep;
		uvStep.x = texelSize.x;
	}
	else {
		edgeUV.x += 0.5 * edge.pixelStep;
		uvStep.y = texelSize.y;
	}

	float edgeLuma = 0.5 * (luma.m + edge.otherLuma);
	float gradientThreshold = 0.25 * edge.lumaGradient;
			
	vec2 uvP = edgeUV + uvStep;
	float lumaDeltaP = GetLuma(uvP) - edgeLuma;
	bool atEndP = abs(lumaDeltaP) >= gradientThreshold;

	int i;

	for (i = 0; i < EXTRA_EDGE_STEPS && !atEndP; i++) 
	{
		uvP += uvStep * edgeStepSizes[i];
		lumaDeltaP = GetLuma(uvP) - edgeLuma;
		atEndP = abs(lumaDeltaP) >= gradientThreshold;
	}
	if (!atEndP) {
		uvP += uvStep * LAST_EDGE_STEP_GUESS;
	}

	vec2 uvN = edgeUV - uvStep;
	float lumaDeltaN = GetLuma(uvN) - edgeLuma;
	bool atEndN = abs(lumaDeltaN) >= gradientThreshold;

	for (i = 0; i < EXTRA_EDGE_STEPS && !atEndN; i++) {
		uvN -= uvStep * edgeStepSizes[i];
		lumaDeltaN = GetLuma(uvN) - edgeLuma;
		atEndN = abs(lumaDeltaN) >= gradientThreshold;
	}
	if (!atEndN) {
		uvN -= uvStep * LAST_EDGE_STEP_GUESS;
	}

	float distanceToEndP, distanceToEndN;
	if (edge.isHorizontal) {
		distanceToEndP = uvP.x - uv.x;
		distanceToEndN = uv.x - uvN.x;
	}
	else {
		distanceToEndP = uvP.y - uv.y;
		distanceToEndN = uv.y - uvN.y;
	}

	float distanceToNearestEnd;
	bool deltaSign;
	if (distanceToEndP <= distanceToEndN) {
		distanceToNearestEnd = distanceToEndP;
		deltaSign = lumaDeltaP >= 0;
	}
	else {
		distanceToNearestEnd = distanceToEndN;
		deltaSign = lumaDeltaN >= 0;
	}

	if (deltaSign == (luma.m - edgeLuma >= 0)) {
		return 0.0;
	}
	else {
		return 0.5 - distanceToNearestEnd / (distanceToEndP + distanceToEndN);
	}
}

void main()
{
	LumaNeighborhood luma = GetLumaNeighborhood(vTexcoord);
	
	if (CanSkipFXAA(luma)) 
	{
		//discard;
		FragColor = GetSource(vTexcoord);
	}
	else
	{
		FXAAEdge edge = GetFXAAEdge(luma);
		float blendFactor = max(
			GetSubpixelBlendFactor(luma), GetEdgeBlendFactor(luma, edge, vTexcoord)
		);
		vec2 blendUV = vTexcoord;
		if (edge.isHorizontal) {
			blendUV.y += blendFactor * edge.pixelStep;
		}
		else {
			blendUV.x += blendFactor * edge.pixelStep;
		}
		FragColor = GetSource(blendUV);
		//FragColor = vec4(vec3(edge.lumaGradient), 1.0);
	}
	
}