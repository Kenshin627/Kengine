#pragma once
#include "../renderPass.h"

class LightingPass : public RenderPass
{
public:
	LightingPass(uint width, uint height);
	~LightingPass() = default;
};