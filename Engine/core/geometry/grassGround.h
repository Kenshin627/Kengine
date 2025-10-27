#pragma once
#include "geometry.h"

class GrassGround :public Geometry
{
public:
	GrassGround(float width, float height, const glm::vec2& subdivision = { 1.0, 1.0 });
	~GrassGround();
	virtual void buildGeometry() override;
private:
	float	  mWidth;
	float	  mHeight;
	glm::vec2 subdivision;
};