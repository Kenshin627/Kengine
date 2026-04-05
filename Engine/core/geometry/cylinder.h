#pragma once
#include "geometry.h"

#define PI 3.1415926535897932384626433832795f

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
	~Cylinder() {};
	virtual void buildGeometry() override;
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