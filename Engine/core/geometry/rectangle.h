#pragma once
#include "geometry.h"

class Rectangle :public Geometry
{
public:
	Rectangle(float width, float height);
	~Rectangle();
	virtual void buildGeometry() override;
private:
	float mWidth;
	float mHeight;
};