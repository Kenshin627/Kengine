#include "core.h"
#include "flatWireframePass.h"
#include "geometry/screenQuad.h"
#include "graphic/program/program.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "scene/scene.h"
#include "scene/renderObject.h"

FlatWireFramePass::FlatWireFramePass(const glm::vec3& faceColor, const glm::vec3& lineColor, float thickness, float smoothing, Renderer* r, const RenderState& state)
	:RenderPass(r, state),
	mFaceColor(faceColor),
	mLineColor(lineColor),
	mThickness(thickness),
	mSmoothing(smoothing)
{
	mProgram = std::make_shared<Program>();
	mProgram->buildFromFiles({
		{ "core/graphic/shaderSrc/flatWireframe/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/flatWireframe/gs.glsl", ShaderType::Geometry },
		{ "core/graphic/shaderSrc/flatWireframe/fs.glsl", ShaderType::Fragment }
		});
	//default to screen, no fbo, no program, no vbo	
	//check if renderTarget to screen, if no build fbo
	if (state.target == RenderTarget::FRAMEBUFFER)
	{
		std::initializer_list<FrameBufferSpecification> spec =
		{
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
				TextureWarpMode::CLAMP_TO_BORDER,
				TextureWarpMode::CLAMP_TO_BORDER,
				TextureFilter::NEAREST,
				TextureFilter::NEAREST
			}
		};
		mFrameBuffer = std::make_unique<FrameBuffer>(glm::vec3{ mSize.x, mSize.y ,0 }, spec);
	}
}

FlatWireFramePass::~FlatWireFramePass()
{
}

void FlatWireFramePass::beginPass()
{
	RenderPass::beginPass();
	mProgram->setUniform("faceColor", mFaceColor);
	mProgram->setUniform("lineColor", mLineColor);
	mProgram->setUniform("thickness", mThickness);
	mProgram->setUniform("smoothing", mSmoothing);
}

void FlatWireFramePass::runPass(Scene* scene)
{	
	for (auto& renderObject : scene->getRenderList())
	{
		renderObject->beginDraw(mProgram.get());
		renderObject->draw();
		renderObject->endDraw(mProgram.get());
	}
}
