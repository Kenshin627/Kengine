#include "gaussianBlur.h"
#include "graphic/program/program.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "geometry/screenQuad.h"
#include "core.h"
#include "scene/scene.h"

GaussianBlur::GaussianBlur(const RenderState& state)
	:RenderPass(state)
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
	std::initializer_list<FrameBufferSpecification> spec =
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
	mDoubleBuffers.push_back(std::make_shared<FrameBuffer>(glm::vec3{ state.width, state.height , 0}, spec));
	mDoubleBuffers.push_back(std::make_shared<FrameBuffer>(glm::vec3{ state.width, state.height , 0}, spec));
}

GaussianBlur::~GaussianBlur()
{
}

void GaussianBlur::beginPass()
{
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
	uint amount = 20;
	Texture* tex;
	for (int i = 0; i < amount; i++)
	{
		mDoubleBuffers[!isHorizontal]->bind();
		updateRenderState();
		if (isFirstIteration)
		{
			tex = mlastPassFrameBuffer[0]->getColorAttachment(1);
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

std::shared_ptr<FrameBuffer> GaussianBlur::getOutputFrameBuffer()
{
	return mDoubleBuffers[1];
}

void GaussianBlur::resize(uint width, uint height)
{
	RenderPass::resize(width, height);
	for (auto& fbo : mDoubleBuffers)
	{
		fbo->resize(width, height);
	}
}
