#pragma once
#include "../renderPass.h"

class LightingPass : public RenderPass
{
public:
	LightingPass(const RenderState& state);
	~LightingPass() = default;
};