#pragma once
#include "../../renderPass.h"

class NonMaxSuppression :public RenderPass
{
public:
	NonMaxSuppression(Renderer* r, const RenderState& state);
	~NonMaxSuppression() = default;
	virtual void beginPass() override;
	virtual void runPass(Scene* scene) override;
};
