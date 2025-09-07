#pragma once
#include "typedef.h"
#include "../renderPass.h"

class GeometryPass :public RenderPass
{
public:
	GeometryPass(uint width, uint height);
	~GeometryPass() = default;
};