#include "defaultPass.h"
#include "scene/scene.h"
#include <graphic/gpuBuffer/frameBuffer.h>

DefaultPass::DefaultPass(const RenderState& state)
	:RenderPass(state)
{
	//default to screen, no fbo, no program, no vbo	
	//check if renderTarget to screen, if no build fbo
	if (state.target == RenderTarget::FRAMEBUFFER)
	{
		std::initializer_list<FrameBufferSpecification> specs =
		{
			{
				AttachmentType::Color,
				TextureInternalFormat::RGBA8,
				TextureDataFormat::RGBA,
				TextureWarpMode::CLAMP_TO_EDGE,
				TextureWarpMode::CLAMP_TO_EDGE,
				TextureFilter::NEAREST,
				TextureFilter::NEAREST
			},
			{
				AttachmentType::DepthStencil,
				TextureInternalFormat::DEPTH24STENCIL8,
				TextureDataFormat::DELTHSTENCIL,
				TextureWarpMode::CLAMP_TO_BORDER,
				TextureWarpMode::CLAMP_TO_BORDER,
				TextureFilter::NEAREST,
				TextureFilter::NEAREST
			}
		};
		mFrameBuffer = std::make_unique<FrameBuffer>(state.width, state.height, specs);
	}
}

DefaultPass::~DefaultPass()
{
}

void DefaultPass::runPass(Scene* scene)
{
	scene->draw();
}
