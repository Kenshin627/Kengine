#include "core.h"
#include "grayScaleEffect.h"
#include "geometry/screenQuad.h"
#include "graphic/program/program.h"
#include "graphic/gpuBuffer/frameBuffer.h"

GrayScaleEffect::GrayScaleEffect(const RenderState& state)
	:RenderPass(state)
{
	//post effect to screen 
	mGeometry = std::make_shared<ScreenQuad>();
	mProgram = std::make_shared<Program>();
	mProgram->buildFromFiles({
		{ "core/graphic/shaderSrc/postProcess/grayScale/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/postProcess/grayScale/fs.glsl", ShaderType::Fragment }
	});
}

void GrayScaleEffect::runPass(Scene* scene)
{
	//get screenMap texuture
	if (!mlastPassFrameBuffer.empty())
	{
		//TODO
		Texture2D* tex = mlastPassFrameBuffer[0]->getColorAttachment(0);
		if (!tex)
		{
			KS_CORE_ERROR("framebuffer attachment at index {0} is null", 0);
			return;
		}
		tex->bind(0);
	}
	//set screen map uniform
	mProgram->setUniform("screenMap", 0);
	mGeometry->draw();
}
