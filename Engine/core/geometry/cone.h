#pragma once
#include "cylinder.h"

class Cone : public Cylinder
{
public:
	Cone(
		float radius = 1, 
		float height = 1, 
		int radialSegments = 32, 
		int heightSegments = 1, 
		bool openEnded = false, 
		float thetaStart = 0, 
		float thetaLength = PI * 2
	);
	~Cone() = default;
};