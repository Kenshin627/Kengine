#include "forwardShadingPass.h"
#include "scene/scene.h"
#include <graphic/gpuBuffer/frameBuffer.h>

ForwardShadingPass::ForwardShadingPass(Renderer* r, const RenderState& state)
	:RenderPass(r, state)
{
	//default to screen, no fbo, no program, no vbo	
	//check if renderTarget to screen, if no build fbo
	if (state.target == RenderTarget::FRAMEBUFFER)
	{
		std::initializer_list<FrameBufferSpecification> spec =
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
				AttachmentType::Depth,
				TextureInternalFormat::DEPTH32,
				TextureDataFormat::DEPTH,
				TextureWarpMode::CLAMP_TO_BORDER,
				TextureWarpMode::CLAMP_TO_BORDER,
				TextureFilter::NEAREST,
				TextureFilter::NEAREST
			}
		};
		mFrameBuffer = std::make_unique<FrameBuffer>(glm::vec3{ mSize.x, mSize.y ,0 }, spec);
	}
}

void ForwardShadingPass::runPass(Scene* scene)
{
	scene->draw();
}
