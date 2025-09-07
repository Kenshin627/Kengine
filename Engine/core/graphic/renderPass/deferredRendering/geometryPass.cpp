#include "geometryPass.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "graphic/program/program.h"
#include "scene/scene.h"
#include "scene/renderObject.h"
#include "material/material.h"

GeometryPass::GeometryPass(const RenderState& state)
	:RenderPass(state)
{
	//GBUFFER
	//|		RGB8  | worldPosition	| 
	//|		RGB8  | worldNormal		|
	//|		RGB8  | diffuse      	|
	//|     RGBA8 | spec + shiness  |
	std::initializer_list<FrameBufferSpecification> specs =
	{
		{
			AttachmentType::Color,
			TextureInternalFormat::RGB16F,
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
			TextureInternalFormat::RGB8,
			TextureDataFormat::RGB,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureFilter::NEAREST,
			TextureFilter::NEAREST
		},
		{
			AttachmentType::Color,
			TextureInternalFormat::RGBA16F,
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
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureWarpMode::CLAMP_TO_EDGE,
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
}

void GeometryPass::runPass(Scene* scene)
{
	for (auto& renderObject : scene->getRenderList())
	{
		renderObject->beginDraw(mProgram.get());
		renderObject->draw();
		renderObject->endDraw(mProgram.get());
	}
}
