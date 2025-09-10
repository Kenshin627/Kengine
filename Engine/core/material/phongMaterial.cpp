#include "phongMaterial.h"
#include "graphic/texture/texture2D/texture2D.h"

PhongMaterial::PhongMaterial(const char* diffuseMap, const char* specularMap, float shiness, const char* normalMap, const char* roughnessMap)
	:mDiffuseMap(std::make_shared<Texture2D>()),
	 mSpecularMap(std::make_shared<Texture2D>()),
	 mNormalMap(std::make_shared<Texture2D>()),
	 mShinessMap(std::make_shared<Texture2D>())
{
	initProgram();
	mDiffuseMap->loadFromFile(diffuseMap);
	mSpecularMap->loadFromFile(specularMap);
	if (normalMap)
	{
		mNormalMap->loadFromFile(normalMap);
	}
	if (roughnessMap)
	{
		mShinessMap->loadFromFile(roughnessMap);
	}
}

PhongMaterial::PhongMaterial(std::shared_ptr<Texture2D> diff, std::shared_ptr<Texture2D> specular, float shiness, std::shared_ptr<Texture2D> normal, std::shared_ptr<Texture2D> roughnessMap)
{
	mDiffuseMap = diff;
	mSpecularMap = specular;
	mShiness = shiness;
	mNormalMap = normal;
	mShinessMap = roughnessMap;
}

void PhongMaterial::setUniforms(Program* p) const
{
	mDiffuseMap->bind(0);
	mSpecularMap->bind(1);
	p->setUniform("diffuseMap", 0);
	p->setUniform("specularMap", 1);

	if (mNormalMap)
	{
		mNormalMap->bind(2);
		p->setUniform("normalMap", 2);
	}

	if (mShinessMap)
	{
		mShinessMap->bind(3);
		p->setUniform("roughnessMap", 2);
	}
}

void PhongMaterial::initProgram()
{
	std::initializer_list<ShaderFile> files = {
		{ "core/graphic/shaderSrc/blinnPhong/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/blinnPhong/fs.glsl",ShaderType::Fragment }
	};
	mProgram->buildFromFiles(files);
}
