#pragma once
#include "Geometry.h"

class Torus : public Geometry
{
public:
	//radius = 1, tube = 0.4, radialSegments = 12, tubularSegments = 48, arc = Math.PI * 2, thetaStart = 0, thetaLength = Math.PI * 2
	Torus(
		float radius = 1.0,
		float tube = 0.4,
		int radialSegments = 12,
		int tubularSegments = 48,
		float arc = PI * 2.0,
		float thetaStart = 0,
		float thetaLength = PI * 2.0
	);
	~Torus() = default;
	virtual void buildGeometry() override;
private:
	float	mRadius;
	float	mTube;
	int		mRadialSegments;
	int		mTubularSegments;
	float	mArc;
	float	mThetaStart;
	float	mThetaLength;
};