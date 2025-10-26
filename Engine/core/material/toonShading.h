#pragma once
#include "material.h"

class Texture2D;
struct ToonShadingSpecification
{
	std::shared_ptr<Texture2D> normalMap { nullptr };
	std::shared_ptr<Texture2D> diffuseMap{ nullptr };
	glm::vec3				   diffuseColor{ 0.5, 0.5, 0.5 };
};

class ToonShading :public Material
{
public:
	ToonShading(const ToonShadingSpecification& spec = ToonShadingSpecification());
	~ToonShading() = default;
	virtual void setUniforms(Program* p) const override;
	virtual void initProgram() override;
private:
	std::shared_ptr<Texture2D> mDiffuseMap;
	std::shared_ptr<Texture2D> mNormalMap;
	glm::vec3				   mDiffuseColor;
	bool					   mHasNormalTex{ false };
	bool					   mHasDiffuseTex{ false };
};