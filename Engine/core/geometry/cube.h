#pragma once
#include "geometry.h"

class Cube :public Geometry
{
public:
	Cube(float width, float length, float height);
	~Cube();
	virtual void buildGeometry() override;
private:
	float mWidth;
	float mLength;
	float mHeight;
};