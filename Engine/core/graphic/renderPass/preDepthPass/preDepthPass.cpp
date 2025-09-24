#include "preDepthPass.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "graphic/program/program.h"
#include "scene/scene.h"
#include "scene/renderObject.h"

PreDepthPass::PreDepthPass(Renderer* r, const RenderState& state)
	:RenderPass(r, state)
{
	std::initializer_list<FrameBufferSpecification> fboSpec =
	{
		//TODO: becaz drawbuffers need a color attachment, furture will remove this color attachment
		{
			AttachmentType::Color,
			TextureInternalFormat::R8,
			TextureDataFormat::R,
			TextureWarpMode::CLAMP_TO_BORDER,
			TextureWarpMode::CLAMP_TO_BORDER,
			TextureFilter::NEAREST,
			TextureFilter::NEAREST,
			{1.0, 1.0, 1.0, 1.0}
		},
		{
			AttachmentType::Depth,
			TextureInternalFormat::DEPTH32,
			TextureDataFormat::DEPTH,
			TextureWarpMode::CLAMP_TO_BORDER,
			TextureWarpMode::CLAMP_TO_BORDER,
			TextureFilter::NEAREST,
			TextureFilter::NEAREST,
			{1.0, 1.0, 1.0, 1.0}
		}
	};
	mFrameBuffer = std::make_shared<FrameBuffer>(glm::vec3{ mSize.x, mSize.y , 0 }, fboSpec);
	//program
	mProgram = std::make_shared<Program>();
	std::initializer_list<ShaderFile> files =
	{
		{ "core/graphic/shaderSrc/preDepthPass/vs.glsl", ShaderType::Vertex   },
		{ "core/graphic/shaderSrc/preDepthPass/fs.glsl", ShaderType::Fragment }
	};
	mProgram->buildFromFiles(files);
}

void PreDepthPass::runPass(Scene* scene)
{
	for (auto& renderObject : scene->getRenderList())
	{
		renderObject->beginDraw(mProgram.get());
		renderObject->draw();
		renderObject->endDraw(mProgram.get());
	}
}
