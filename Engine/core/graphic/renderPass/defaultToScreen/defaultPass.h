#pragma once
#include "../renderPass.h"

class DefaultPass :public RenderPass
{
public:
	DefaultPass(uint width, uint height);
	~DefaultPass();
	virtual void runPass(Scene* scene) override;
};