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

	void setBlockSearchSamples(int sample);
	void setLightRadiusUV(float lightUVSize);
	void setLightNear(float lightNear);
	void setLightFar(float lightFar);
	void setPcfSamples(int sample);

	void updateLightViewMatrix();
private:
private:
	std::unique_ptr<UniformBuffer> mLightMatricesBuffer;
	int							   mBlockerSearchSamples{ 128 };
	float						   mLightRadiusUV{ 0.005f };
	float						   mLightNear{};
	float						   mLightFar{};
	int							   mPcfSamples{ 128 };
};

