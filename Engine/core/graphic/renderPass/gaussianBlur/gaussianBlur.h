#pragma once
#include "../renderPass.h"

class GaussianBlur :public RenderPass
{
public:
	GaussianBlur(const RenderState& state);
	~GaussianBlur();
	virtual void beginPass();
	virtual void runPass(Scene* scene) override;
	virtual void endPass() override;
	FrameBuffer* getOutputFrameBuffer();
	virtual void resize(uint width, uint height) override;
private:
	std::vector<std::shared_ptr<FrameBuffer>> mDoubleBuffers;
};