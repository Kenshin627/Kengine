#include "phongMaterial.h"
#include "graphic/texture/texture2D/texture2D.h"

PhongMaterial::PhongMaterial(const char* diffuseMap, const char* specularMap, float shiness)
	:mDiffuseMap(std::make_shared<Texture2D>()),
	 mSpecularMap(std::make_shared<Texture2D>()),
	 mShiness(shiness)
{
	initProgram();
	mDiffuseMap->loadFromFile(diffuseMap);
	mSpecularMap->loadFromFile(specularMap);
}

void PhongMaterial::setUniforms(Program* p) const
{
	mDiffuseMap->bind(0);
	mSpecularMap->bind(1);
	p->setUniform("diffuseMap", 0);
	p->setUniform("specularMap", 1);
	p->setUniform("shiness", mShiness);
}

void PhongMaterial::initProgram()
{
	std::initializer_list<ShaderFile> files = {
		{ "core/graphic/shaderSrc/blinnPhong/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/blinnPhong/fs.glsl",ShaderType::Fragment }
	};
	mProgram->buildFromFiles(files);
}
