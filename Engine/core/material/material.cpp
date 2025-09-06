#include "material.h"
#include "../graphic/program/program.h"
Material::Material()
	:mProgram(std::make_unique<Program>())
{
	
}

void Material::bind() const
{
	mProgram->bind();
}

void Material::unBind() const
{
	mProgram->unBind();
}

void Material::beginDraw()
{
	mProgram->bind();
	setUniforms(mProgram.get());
}

void Material::endDraw()
{
	mProgram->unBind();
}

void Material::initProgram() {}