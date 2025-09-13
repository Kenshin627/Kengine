#pragma once
#include "material.h"

class Texture2D;
struct BlinnPhongMaterialSpecification
{
	std::shared_ptr<Texture2D> diffuseMap		{ nullptr		};
	std::shared_ptr<Texture2D> specularMap		{ nullptr		};
	std::shared_ptr<Texture2D> normalMap		{ nullptr		};
	std::shared_ptr<Texture2D> shinessMap		{ nullptr		};
	glm::vec3				   diffuseColor		{ 0.5, 0.5, 0.5 };
	glm::vec3				   specularColor	{ 1.0, 1.0, 1.0 };
	float					   shiness			{ 0.128f		};
};
class PhongMaterial : public Material
{
public:
	PhongMaterial(const BlinnPhongMaterialSpecification& spec);
	virtual ~PhongMaterial() = default;
	PhongMaterial(const PhongMaterial& obj) = delete;
	PhongMaterial(PhongMaterial&& obj) = delete;
	PhongMaterial& operator=(const PhongMaterial& obj) = delete;
	virtual void setUniforms(Program* p) const override;
	virtual void initProgram() override;
private:
	std::shared_ptr<Texture2D> mDiffuseMap;
	std::shared_ptr<Texture2D> mSpecularMap;
	std::shared_ptr<Texture2D> mNormalMap;
	std::shared_ptr<Texture2D> mShinessMap;
	glm::vec3				   mDiffuseColor;
	glm::vec3				   mSpecularColor;
	float					   mShiness;
	bool					   mHasDiffuseTex { false };
	bool					   mHasSpecularTex{ false };
	bool					   mHasNormalTex  { false };
	bool					   mHasShinessTex { false };
};