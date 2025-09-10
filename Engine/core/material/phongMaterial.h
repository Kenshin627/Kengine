#pragma once
#include "material.h"

class Texture2D;
class PhongMaterial : public Material
{
public:
	PhongMaterial(const char* diffuseMap, const char* specularMap, float shiness = 128.0f, const char* normalMap = nullptr, const char* roughnessMap = nullptr);
	PhongMaterial(std::shared_ptr<Texture2D> diff, std::shared_ptr<Texture2D> specular, float shiness = 128.0, std::shared_ptr<Texture2D> normal = nullptr, std::shared_ptr<Texture2D> roughnessMap = nullptr);	virtual ~PhongMaterial() = default;
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
	float					   mShiness;
};