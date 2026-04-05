#pragma once
#include "Geometry.h"

class TorusKnot : public Geometry
{
public:
	TorusKnot(
		float radius = 1.0,
		float tube = 0.4,
		int tubularSegments = 48,
		int radialSegments = 12,
		int p = 2,
		int q = 3
	);
	~TorusKnot() = default;
	virtual void buildGeometry() override;
private:
	void calculatePositionOnCurve(float u, glm::vec3& position);
private:
	float	mRadius;
	float	mTube;
	int		mTubularSegments;
	int		mRadialSegments;
	int		mP;
	int		mQ;
};