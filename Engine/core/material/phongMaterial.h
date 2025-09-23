#pragma once
#include "material.h"

class Texture2D;
struct BlinnPhongMaterialSpecification
{
	std::shared_ptr<Texture2D> diffuseMap		{ nullptr		};
	std::shared_ptr<Texture2D> specularMap		{ nullptr		};
	std::shared_ptr<Texture2D> normalMap		{ nullptr		};
	std::shared_ptr<Texture2D> shinessMap		{ nullptr		};
	std::shared_ptr<Texture2D> heightMap		{ nullptr		};
	std::shared_ptr<Texture2D> alphaMap			{ nullptr		};
	glm::vec3				   diffuseColor		{ 0.5, 0.5, 0.5 };
	glm::vec3				   specularColor	{ 1.0, 1.0, 1.0 };
	glm::vec3				   emissiveColor    { 0, 0, 0		};
	float					   shiness			{ 128.0f		};
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
	void setDiffuseColor(const glm::vec3& col);
	void setEmissiveColor(const glm::vec3& col);
private:
	std::shared_ptr<Texture2D> mDiffuseMap;
	std::shared_ptr<Texture2D> mSpecularMap;
	std::shared_ptr<Texture2D> mNormalMap;
	std::shared_ptr<Texture2D> mShinessMap;
	std::shared_ptr<Texture2D> mHeightMap;
	std::shared_ptr<Texture2D> mAlphaMap;
	glm::vec3				   mDiffuseColor;
	glm::vec3				   mSpecularColor;
	glm::vec3				   mEmissiveColor;
	float					   mShiness;
	bool					   mHasDiffuseTex { false };
	bool					   mHasSpecularTex{ false };
	bool					   mHasNormalTex  { false };
	bool					   mHasShinessTex { false };
	bool					   mhasHeighTex   { false };
	bool					   mIsEmissive    { false };
	bool					   mHasAlphaTex   { false };
};