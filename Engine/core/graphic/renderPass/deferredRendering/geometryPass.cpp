#include "geometryPass.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "graphic/program/program.h"

GeometryPass::GeometryPass(const RenderState& state)
	:RenderPass(state)
{
	//GBUFFER
	//|		RGB8  | worldPosition	| 
	//|		RGB8  | worldNormal		|
	//|		RGBA8 | diffuse + spec	|
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
			AttachmentType::Color,
			TextureInternalFormat::RGB8,
			TextureDataFormat::RGB,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureFilter::NEAREST,
			TextureFilter::NEAREST
		},
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
	mProgram = std::make_unique<Program>();
	mProgram->buildFromFiles({
		{ "core/graphic/shaderSrc/deferredRendering/geometryPassShader/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/deferredRendering/geometryPassShader/fs.glsl", ShaderType::Fragment }
	});

	//set renderState
	//RenderState state;
	//state.width  = width;
	//state.height =  height;
	//state.viewport.z = width;
	//state.viewport.w = height;
	//state.target = RenderTarget::FRAMEBUFFER;
	//setRenderState(state);
}
