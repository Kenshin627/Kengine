#pragma once
#include "../renderPass.h"

class DefaultPass :public RenderPass
{
public:
	DefaultPass(const RenderState& state);
	~DefaultPass();
	virtual void runPass(Scene* scene) override;
};