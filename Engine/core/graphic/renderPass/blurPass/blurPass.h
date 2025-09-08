#pragma once
#include "../renderPass.h"

class BlurPass :public RenderPass
{
public:
	BlurPass(uint radius, const RenderState& state);
	~BlurPass();
	void setBlurRadius(uint radius);
	uint getBlurRadius() const;
	virtual void beginPass() override;
	virtual void runPass(Scene* scene) override;
private:
	uint mRadius;
};