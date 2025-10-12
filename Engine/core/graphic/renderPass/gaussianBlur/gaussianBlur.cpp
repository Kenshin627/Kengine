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
	generateKernel();
}

GaussianBlur::~GaussianBlur()
{
}

void GaussianBlur::beginPass()
{
	RenderPass::beginPass();
	mProgram->setUniform("scale", mSpec.Scale);
	mProgram->setUniform("strength", mSpec.Strength);
	mProgram->setUniform("kernels", MAX_HALF_KERNEL_SIZE, mKernels.data());
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
			tex = mPrevPass->getCurrentFrameBuffer()->getColorAttachment(0);
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

void GaussianBlur::setSigma(float s)
{
	if (mSpec.Sigma != s)
	{
		mSpec.Sigma = s;
		generateKernel();
	}
}

float GaussianBlur::getSigma() const
{
	return mSpec.Sigma;
}

bool GaussianBlur::checkFrameBuffer()
{
	for (auto& fbo : mDoubleBuffers)
	{
		if (!fbo || !fbo->isComplete())
		{
			return false;
		}
	}
	return true;
}

void GaussianBlur::generateKernel()
{
	mKernels.clear();
	mKernels.reserve(MAX_HALF_KERNEL_SIZE);
	computeKernelSize();
	float result = 0.0f;
	float sum = 0.0f;

	for (int i = 0; i < MAX_HALF_KERNEL_SIZE; i++)
	{
		if (i == 0) 
		{
			mKernels.push_back(1.0f);
		}
		else
		{
			mKernels.push_back(0.0f);
		}
	}

	if (mSpec.Sigma == 0)
	{
		return;
	}
	
	float SS2 = mSpec.Sigma * mSpec.Sigma * 2;
	sum = 1;
	for (int i = 1; i < mSpec.HalfKernelSize; ++i)
	{
		result = std::exp(-(i * i) / SS2);
		mKernels[i] = result;
		sum += result * 2;
	}

	// normalize kernel
	// make sum of all elements in kernel to 1
	for (int i = 0; i < mSpec.HalfKernelSize; ++i)
	{
		mKernels[i] /= sum;
	}
}

void GaussianBlur::computeKernelSize()
{
	// determine size of kernel (odd #)
	// sigma = 0          : 1
	// 0.0 <  sigma < 0.5 : 3
	// 0.5 <= sigma < 1.0 : 5
	// 1.0 <= sigma < 1.5 : 7
	// 1.5 <= sigma < 2.0 : 9
	// 2.0 <= sigma < 2.5 : 11
	// 2.5 <= sigma < 3.0 : 13 ...
	if (mSpec.Sigma <= 0)
	{
		mSpec.HalfKernelSize = 1;
	}
	else
	{
		uint kernelSize = 2 * std::round(2 * mSpec.Sigma) + 3;
		mSpec.HalfKernelSize = std::floor(kernelSize / 2) + 1;
		if (mSpec.HalfKernelSize > MAX_HALF_KERNEL_SIZE)
		{ 
			mSpec.HalfKernelSize = MAX_HALF_KERNEL_SIZE;
		}
	}
}
