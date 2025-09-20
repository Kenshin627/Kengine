#pragma once
#include "../renderPass.h"

class CascadeShadowMapPass;
class LightingPass : public RenderPass
{
public:
	LightingPass(const RenderState& state);
	~LightingPass() = default;
	virtual void runPass(Scene* scene) override;
	virtual void beginPass() override;
	void setCascadedShadowMapPass(CascadeShadowMapPass* pass);
private:
	CascadeShadowMapPass* mCascadedShadowMapPass;
};