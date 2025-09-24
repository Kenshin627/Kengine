#pragma once
#include "typedef.h"
#include "../renderPass.h"

class GeometryPass :public RenderPass
{
public:
	GeometryPass(Renderer* r, const RenderState& state);
	~GeometryPass() = default;
	virtual void beginPass() override;
	virtual void runPass(Scene* scene) override;
	void setPreDepthFrameBuffer(FrameBuffer* preDepthFBO);
private:
	FrameBuffer* mPreDepthFBO;
};