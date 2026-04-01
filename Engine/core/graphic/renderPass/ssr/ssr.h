#pragma once
#include "../renderPass.h"
class SSR :public RenderPass
{
public:
	SSR(Renderer* r, const RenderState& state);
	~SSR();
	virtual void beginPass() override;
	virtual void runPass(Scene* scene) override;
	virtual void endPass() override;
private:
	
};

