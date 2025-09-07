#pragma once
#include "../../renderPass.h"

class GrayScaleEffect :public RenderPass
{
public:
	//disable depthTest
	GrayScaleEffect(const RenderState& state);
	~GrayScaleEffect() = default;
	virtual void runPass(Scene* scene) override;
};