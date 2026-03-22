#pragma once
#include "../renderPass.h"
class FlatWireFramePass :public RenderPass
{
public:
	FlatWireFramePass(const glm::vec3& faceColor, const glm::vec3& lineColor, float thickness, float smoothing, Renderer* r, const RenderState& state);
	~FlatWireFramePass();
	virtual void beginPass() override;
	virtual void runPass(Scene* scene) override;
private:
	//uniform vec3 faceColor;
	//uniform vec3 lineColor;
	//uniform float thickness;
	//uniform float smoothing;
	glm::vec3 mFaceColor	{ 0.8f, 0.8f, 0.8f };
	glm::vec3 mLineColor	{ 0.2f, 0.2f, 0.2f };
	float mThickness		{ 0.2f };
	float mSmoothing		{ 0.01f };
};

