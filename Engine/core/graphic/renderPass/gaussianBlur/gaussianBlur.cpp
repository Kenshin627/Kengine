#include "gaussianBlur.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "graphic/renderer/renderer.h"
#include "graphic/program/program.h"
#include "geometry/screenQuad.h"
#include "scene/scene.h"
#include "core.h"

GaussianBlur::GaussianBlur(const GaussianBlurSpecification& spec, Renderer* r, const RenderState& state)
	:RenderPass(r, state),
	 mSpec(spec)
{
	//program
	mProgram = std::make_shared<Program>();
	std::initializer_list<ShaderFile> files =
	{
		{ "core/graphic/shaderSrc/gaussianBlur/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/gaussianBlur/fs.glsl", ShaderType::Fragment }
	};
	mProgram->buildFromFiles(files);
	//quad
	//2 * frambuffer
	std::initializer_list<FrameBufferSpecification> fboSpec =
	{
		{
			AttachmentType::Color,
			TextureInternalFormat::RGBA16F,
			TextureDataFormat::RGBA,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureFilter::LINEAR,
			TextureFilter::LINEAR
		}
	};
	mDoubleBuffers.push_back(std::make_shared<FrameBuffer>(glm::vec3{ mSize.x, mSize.y ,0 }, fboSpec));
	mDoubleBuffers.push_back(std::make_shared<FrameBuffer>(glm::vec3{ mSize.x, mSize.y ,0 }, fboSpec));
}

GaussianBlur::~GaussianBlur()
{
}

void GaussianBlur::beginPass()
{
	RenderPass::beginPass();
	mProgram->setUniform("scale", mSpec.Scale);
	mProgram->setUniform("strength", mSpec.Strength);
}

void GaussianBlur::runPass(Scene* scene)
{
	ScreenQuad* quad = scene->getScreenQuad();
	if (!quad)
	{
		KS_CORE_ERROR("pass screen quad is null");
		return;
	}

	bool isHorizontal = true;
	bool isFirstIteration = true;
	mProgram->bind();

	Texture* tex;
	for (int i = 0; i < mSpec.Amount; i++)
	{
		mDoubleBuffers[!isHorizontal]->bind();
		updateRenderState();
		if (isFirstIteration)
		{
			//TODO
			tex = mPrevPass->getCurrentFrameBuffer()->getColorAttachment(1);
			isFirstIteration = false;
		}
		else
		{
			tex = mDoubleBuffers[isHorizontal]->getColorAttachment(0);
		}
		tex->bind(0);
		mProgram->setUniform("screenMap", 0);
		mProgram->setUniform("isHorizontal", isHorizontal);
		isHorizontal = !isHorizontal;
		quad->beginDraw();
		quad->draw();
		quad->endDraw();
	}
}

void GaussianBlur::endPass()
{
}

FrameBuffer* GaussianBlur::getOutputFrameBuffer()
{
	return mDoubleBuffers[1].get();
}

void GaussianBlur::resize(uint width, uint height)
{
	RenderPass::resize(width, height);
	for (auto& fbo : mDoubleBuffers)
	{
		fbo->resize(width, height);
	}
}

void GaussianBlur::setGaussianBlurAmount(uint amount)
{
	mSpec.Amount = amount;
}

uint GaussianBlur::getGussianBlurAmount() const
{
	return mSpec.Amount;
}

void GaussianBlur::setGaussianBlurScale(float s)
{
	mSpec.Scale = s;
}

float GaussianBlur::getGussianBlurScale() const
{
	return mSpec.Scale;
}

void GaussianBlur::setGaussianBlurStrength(float strength)
{
	mSpec.Strength = strength;
}

float GaussianBlur::getGussianBlurStrength() const
{
	return mSpec.Strength;
}
