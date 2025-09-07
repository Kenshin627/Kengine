#pragma once
#include "typedef.h"
#include "../renderPass.h"

class GeometryPass :public RenderPass
{
public:
	GeometryPass(const RenderState& state);
	~GeometryPass() = default;
};