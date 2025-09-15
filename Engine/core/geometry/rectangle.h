#pragma once
#include "geometry.h"

class Rectangle :public Geometry
{
public:
	Rectangle(float width, float height, const glm::vec2& uvScale = {1.0, 1.0});
	~Rectangle();
	virtual void buildGeometry() override;
private:
	float	  mWidth;
	float	  mHeight;
	glm::vec2 mUVscale;
};