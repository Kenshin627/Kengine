#pragma once
#include "geometry.h"

class Sphere :public Geometry
{
public:
	Sphere(float r, float segment, float ring);
	~Sphere();
	virtual void buildGeometry() override;
private:
	float mRadius;
	float mSegment;
	float mRing;
};