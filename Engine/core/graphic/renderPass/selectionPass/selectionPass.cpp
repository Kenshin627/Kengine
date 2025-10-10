#include "selectionPass.h"
#include "graphic/program/program.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "scene/scene.h"
#include "scene/renderObject.h"

SelectionPass::SelectionPass(Renderer* s, const RenderState& state)
	:RenderPass(s, state)
{
	mProgram = std::make_shared<Program>();
	std::initializer_list<ShaderFile> shaders =
	{
		{ "core/graphic/shaderSrc/selectionPass/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/selectionPass/fs.glsl", ShaderType::Fragment }
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
		}
	};
	mFrameBuffer = std::make_unique<FrameBuffer>(glm::vec3{ mSize.x, mSize.y , 0 }, specs);
}

SelectionPass::~SelectionPass()
{
}

void SelectionPass::beginPass()
{
	RenderPass::beginPass();
}

void SelectionPass::runPass(Scene* scene)
{
	for (size_t i = 0; i < scene->getRenderList().size(); i++)
	{
		auto& renderObject = scene->getRenderList()[i];
		if (renderObject->getType() == RenderObjectType::Mesh)
		{
			renderObject->beginDraw(mProgram.get());
			renderObject->draw();
			renderObject->endDraw(mProgram.get());
		}
	}
}
~