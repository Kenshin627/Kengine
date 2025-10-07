#pragma once
#include "../../renderPass.h"

class DoubleThreshold :public RenderPass
{
public:
	DoubleThreshold(Renderer* r, const RenderState& state);
	~DoubleThreshold() = default;
	virtual void beginPass() override;
	virtual void runPass(Scene* scene) override;
	void setLowThreshold(float lowValue);
	float getLowThreshold() const;
	void setHighThreshold(float highValue);
	float getHighThreshold() const;
private:
	float mLowThreshold { 0.1f };
	float mHighThreshold{ 0.3f };
};