#pragma once
#include "../renderPass.h"


class Texture;
class BloomPass :public RenderPass
{
public:
	BloomPass(Renderer* r, const RenderState& state);
	~BloomPass();
	virtual void beginPass() override;
	virtual void runPass(Scene* scene) override;
	Texture* getHDRTexture() const;
	Texture* getLDRTexture() const;
	void setThresholdMin(float t) { mThresholdMin = t; }
	float getThresholdMin() const { return mThresholdMin; }
	void setThresholdMax(float t) { mThresholdMax = t; }
	float getThresholdMax() const { return mThresholdMax; }
private:
	float mThresholdMin { 0.8f };
	float mThresholdMax { 1.2f };
};