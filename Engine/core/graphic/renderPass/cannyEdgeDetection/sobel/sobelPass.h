#pragma once
#include "../../renderPass.h"

class SobelPass :public RenderPass
{
public:
	SobelPass(Renderer* r, const RenderState& state);
	~SobelPass() = default;
	virtual void beginPass() override;
	virtual void runPass(Scene* scene) override;
};
