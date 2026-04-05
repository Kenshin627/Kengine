#pragma once
#include "Geometry.h"

class Ring : public Geometry
{
public:
	Ring(
		float innerRadius = 0.5, 
		float outerRadius = 1, 
		int thetaSegments = 32, 
		int phiSegments = 1,
		float thetaStart = 0, 
		float thetaLength = PI * 2
	);
	~Ring() = default;
	virtual void buildGeometry() override;
private:
	float mInnerRadius;
	float mOuterRadius;
	int   mThetaSegments;
	int   mPhiSegments;
	float mThetaStart;
	float mThetaLength;
};