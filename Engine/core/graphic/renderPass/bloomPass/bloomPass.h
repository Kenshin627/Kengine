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
private:
};