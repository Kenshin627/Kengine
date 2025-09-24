#pragma once
#include "../renderPass.h"

class ForwardShadingPass :public RenderPass
{
public:
	ForwardShadingPass(Renderer* r, const RenderState& state);
	~ForwardShadingPass() = default;
	virtual void runPass(Scene* scene) override;
};