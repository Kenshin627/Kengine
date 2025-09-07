#pragma once
#include "geometry.h"

class ScreenQuad :public Geometry
{
public:
	ScreenQuad();
	~ScreenQuad();
	virtual void buildGeometry() override;
};