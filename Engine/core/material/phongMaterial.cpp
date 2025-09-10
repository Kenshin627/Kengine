#include "phongMaterial.h"
#include "graphic/texture/texture2D/texture2D.h"

PhongMaterial::PhongMaterial(const char* diffuseMap, const char* specularMap, const char* normalMap, const char* roughnessMap)
	:mDiffuseMap(std::make_shared<Texture2D>()),
	 mSpecularMap(std::make_shared<Texture2D>()),
	 mNormalMap(std::make_shared<Texture2D>()),
	 mShinessMap(std::make_shared<Texture2D>())
{
	initProgram();
	mDiffuseMap->loadFromFile(diffuseMap);
	mSpecularMap->loadFromFile(specularMap);
	mNormalMap->loadFromFile(normalMap);
	mShinessMap->loadFromFile(roughnessMap);
}

void PhongMaterial::setUniforms(Program* p) const
{
	mDiffuseMap->bind(0);
	mSpecularMap->bind(1);
	mNormalMap->bind(2);
	mShinessMap->bind(3);
	p->setUniform("diffuseMap", 0);
	p->setUniform("specularMap", 1);
	p->setUniform("normalMap", 2);
	p->setUniform("roughnessMap", 2);
}

void PhongMaterial::initProgram()
{
	std::initializer_list<ShaderFile> files = {
		{ "core/graphic/shaderSrc/blinnPhong/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/blinnPhong/fs.glsl",ShaderType::Fragment }
	};
	mProgram->buildFromFiles(files);
}
