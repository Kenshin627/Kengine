#pragma once
#include "../renderPass.h"
class FXAA :public RenderPass
{
public:
	FXAA(Renderer* r, const RenderState& state);
	~FXAA();
	virtual void beginPass() override;
	virtual void runPass(Scene* scene) override;
private:
	//x: fixThreshold	    [Range(0.0312f, 0.0833f)
	//y: relativeThreshold  [Range(0.063f, 0.333f)]
	//z: blendFactor        [Range(0.0f, 1.0f)]
	glm::vec4 mFxaaConfig{ 0.0312f, 0.063f, 1.0f, 0.0f };
	//0: low 1: medium 2:high
	int		  mFxaaQuality{1};
	glm::vec2 mTexelSize;
};

