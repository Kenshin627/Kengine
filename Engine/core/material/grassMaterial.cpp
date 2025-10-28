#include "grassMaterial.h"
#include "graphic/texture/texture2D/texture2D.h"

GrassMaterial::GrassMaterial(const GrassMaterialSpecification& spec)
	:Material(),
	mSpec(spec)
{
	mWindTexture = std::make_unique<Texture2D>();
	mWindTexture->loadFromFile(mSpec.windTexturePath.c_str());
	initProgram();
}

void GrassMaterial::setUniforms(Program* p) const
{
	p->setUniform("bladeWidth", mSpec.bladeWidth);
	p->setUniform("bladeWidthRandom", mSpec.bladeWidthRandom);
	p->setUniform("bladeHeight", mSpec.bladeHeight);
	p->setUniform("bladeHeightRandom", mSpec.bladeHeightRandom);
	p->setUniform("bladeBendRandom", mSpec.bladeBendRandom);
	p->setUniform("bladeForward", mSpec.bladeForward);
	p->setUniform("bladeCurve", mSpec.bladeCurve);
	p->setUniform("bladeTopColor", mSpec.bladeTopColor);
	p->setUniform("bladeBottomColor", mSpec.bladeBottomColor);
	p->setUniform("windScaleOffset", mSpec.windScaleOffset);
	p->setUniform("windFrequency", mSpec.windFrequency);
	p->setUniform("windStrength", mSpec.windStrength);
	mWindTexture->bind(0);
	p->setUniform("windTexture", 0);
	//p->setUniform("time", time);
}

void GrassMaterial::initProgram()
{
	std::initializer_list<ShaderFile> files = {
		{ "core/graphic/shaderSrc/grass/vs.glsl", ShaderType::Vertex   },
		{ "core/graphic/shaderSrc/grass/tcs.glsl", ShaderType::TessControl   },
		{ "core/graphic/shaderSrc/grass/tes.glsl", ShaderType::TessEvaluation   },
		{ "core/graphic/shaderSrc/grass/gs.glsl", ShaderType::Geometry },
		{ "core/graphic/shaderSrc/grass/fs.glsl",ShaderType::Fragment  }
	};
	mProgram->buildFromFiles(files);
}

void GrassMaterial::endDraw()
{
	Material::endDraw();
	mWindTexture->unBind();
}
