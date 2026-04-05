#pragma once
#include "geometry.h"

class Capsule :public Geometry
{
public:
	Capsule(float radius = 1, float height = 1, int capSegments = 4, int radialSegments = 8, int heightSegments = 1);
	~Capsule() {};
	virtual void buildGeometry() override;
private:
	float mRadius;
	float mHeight;
	int   mCapSegments;
	int   mRadialSegments;
	int   mHeightSegments;
};