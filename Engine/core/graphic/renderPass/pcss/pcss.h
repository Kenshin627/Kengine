#pragma once
#include "../renderPass.h"
class PCSS :public RenderPass
{
public:
	PCSS(Renderer* r, const RenderState& state);
	~PCSS();
	virtual void beginPass() override;
	virtual void runPass(Scene* scene) override;
	int getBlockSearchSamples() const;
	float getLightRadiusUV() const;
	float getLightNear() const;
	float getLightFar() const;
	int getPcfSamples() const;
	void updateLightViewMatrix();
private:
private:
	std::unique_ptr<UniformBuffer> mLightMatricesBuffer;
	//uniform int   uBlockerSearchSamples;
	//uniform float uLightRadiusUV;
	//uniform float uLightNear;
	//uniform float uLightFar;
	//uniform int   uPcfSamples;
	int		mBlockerSearchSamples{ 128 };
	float	mLightRadiusUV{ 0.005f };
	float	mLightNear{};
	float	mLightFar{};
	int		mPcfSamples{ 128 };
};

