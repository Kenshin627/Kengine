#pragma once
#include "geometry.h"

class Cylinder :public Geometry
{
public:
	Cylinder(
		float radiusTop = 1, 
		float radiusBottom = 1, 
		float height = 1, 
		int radialSegments = 32, 
		int heightSegments = 1, 
		bool openEnded = false, 
		float thetaStart = 0, 
		float thetaLength = PI * 2);
	virtual ~Cylinder() {};
private:
	void generateTorso(int& index, std::vector<Vertex>& vertices, std::vector<int>& indices);
	void generateCap(bool top, int& index, std::vector<Vertex>& vertices, std::vector<int>& indices);
private:
	float mRadiusTop;
	float mRadiusBottom;
	float mHeight;
	int   mRadialSegments;
	int   mHeightSegments;
	bool  mOpenEnded;
	float mThetaStart;
	float mThetaLength;
};