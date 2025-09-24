#include "geometryPass.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "graphic/program/program.h"
#include "scene/scene.h"
#include "scene/renderObject.h"
#include "material/material.h"

GeometryPass::GeometryPass(Renderer* r, const RenderState& state)
	:RenderPass(r, state)
{
	//GBUFFER
	//|		RGB16F     | viewPosition				     |       viewPosition         |
	//|		RGB16F     | viewNormal					     |       viewNormal           |
	//|		RGBA16F    | diffuse/emissive + isEmissive   |       albedo               |
	//|     RGBA16F    | spec + shiness				     |       metallic + roughness |
	//|     R8         | materialType 0				     |       materialType 1       |
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
			TextureInternalFormat::RGB16F,
			TextureDataFormat::RGB,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureFilter::NEAREST,
			TextureFilter::NEAREST
		},
		{
			AttachmentType::Color,
			TextureInternalFormat::RGBA16F,
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
			AttachmentType::Color,
			TextureInternalFormat::R8,
			TextureDataFormat::R,
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
	mFrameBuffer = std::make_unique<FrameBuffer>(glm::vec3{ mSize.x, mSize.y ,0 }, specs);
	mProgram = std::make_unique<Program>();
	mProgram->buildFromFiles({
		{ "core/graphic/shaderSrc/deferredRendering/geometryPassShader/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/deferredRendering/geometryPassShader/fs.glsl", ShaderType::Fragment }
	});
}

void GeometryPass::beginPass()
{
	RenderPass::beginPass();
	//blit fbo, copy depth attachment
	//glBlitNamedFramebuffer(mPreDepthFBO->id(), mFrameBuffer->id(), 0, 0, mSize.x, mSize.y, 0, 0, mSize.x, mSize.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

void GeometryPass::runPass(Scene* scene)
{
	//TODO
	mProgram->setUniform("heightMapScale", scene->getHeightMapScale());
	for (auto& renderObject : scene->getRenderList())
	{
		renderObject->beginDraw(mProgram.get());
		renderObject->draw();
		renderObject->endDraw(mProgram.get());
	}
}

void GeometryPass::setPreDepthFrameBuffer(FrameBuffer* preDepthFBO)
{
	mPreDepthFBO = preDepthFBO;
}
