#pragma once
#include "../renderPass.h"
class ToneMapping :public RenderPass
{
public:
	ToneMapping(float explosure, const RenderState& state);
	~ToneMapping();
	virtual void beginPass() override;
	virtual void runPass(Scene* scene) override;
private:
	float mExposure;
};

