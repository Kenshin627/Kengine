#include "blurPass.h"
#include "graphic/const.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "graphic/program/program.h"
#include "scene/scene.h"
#include "core.h"

BlurPass::BlurPass(uint radius, const RenderState& state)
	:RenderPass(state),
	 mRadius(radius)
{
	mProgram = std::make_shared<Program>();
	std::initializer_list<ShaderFile> shaders =
	{
		{ "core/graphic/shaderSrc/ssao/blur/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/ssao/blur/fs.glsl", ShaderType::Fragment }
	};
	mProgram->buildFromFiles(shaders);

	std::initializer_list<FrameBufferSpecification> specs =
	{
		{
			AttachmentType::Color,
			TextureInternalFormat::RGB8,
			TextureDataFormat::RGB,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureFilter::NEAREST,
			TextureFilter::NEAREST
		},
		{
			AttachmentType::Depth,
			TextureInternalFormat::DEPTH32,
			TextureDataFormat::DEPTH,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureFilter::NEAREST,
			TextureFilter::NEAREST
		}
	};
	mFrameBuffer = std::make_unique<FrameBuffer>(state.width, state.height, specs);
}

BlurPass::~BlurPass()
{
}

void BlurPass::setBlurRadius(uint radius)
{
	if (mRadius != radius)
	{
		mRadius = radius;
	}
}

uint BlurPass::getBlurRadius() const
{
	return mRadius;
}

void BlurPass::beginPass()
{
	RenderPass::beginPass();
	//set blur Radius
	mProgram->setUniform("blurRadius", mRadius);
	//set lastFrame ssaoMap
	Texture2D* ssaoMap = mlastPassFrameBuffer[0]->getColorAttachment(0);
	ssaoMap->bind(0);
	mProgram->setUniform("ssaoMap", 0);
}

void BlurPass::runPass(Scene* scene)
{
	ScreenQuad* quad = scene->getScreenQuad();
	if (!quad)
	{
		KS_CORE_ERROR("pass screen quad is null");
		return;
	}
	quad->beginDraw();
	quad->draw();
	quad->endDraw();
}
