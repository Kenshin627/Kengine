#pragma once
#include "material.h"

class Texture2D;
class PhongMaterial : public Material
{
public:
	PhongMaterial(const char* diffuseMap, const char* specularMap, float shiness = 32.0f);
	virtual ~PhongMaterial() = default;
	PhongMaterial(const PhongMaterial& obj) = delete;
	PhongMaterial(PhongMaterial&& obj) = delete;
	PhongMaterial& operator=(const PhongMaterial& obj) = delete;
	virtual void setUniforms(Program* p) const override;
	virtual void initProgram() override;
private:
	std::shared_ptr<Texture2D> mDiffuseMap;
	std::shared_ptr<Texture2D> mSpecularMap;
	float					   mShiness;
};