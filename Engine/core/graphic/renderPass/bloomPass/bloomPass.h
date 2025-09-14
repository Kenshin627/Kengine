#pragma once
#include "../renderPass.h"

class BloomPass :public RenderPass
{
public:
	BloomPass(const RenderState& state);
	~BloomPass();
	virtual void beginPass() override;
	virtual void runPass(Scene* scene) override;
private:
};