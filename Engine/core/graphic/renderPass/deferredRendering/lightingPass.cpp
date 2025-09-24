#include "graphic/renderPass/cascadeShadowMapPass/cascadeShadowMapPass.h"
#include "graphic/texture/texture3D/texture3D.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "graphic/renderer/renderer.h"
#include "graphic/program/program.h"
#include "geometry/screenQuad.h"
#include "lightingPass.h"
#include "scene/scene.h"
#include "core.h"

LightingPass::LightingPass(Renderer* r, const RenderState& state)
	:RenderPass(r, state)
{
	mProgram = std::make_unique<Program>();
	mProgram->buildFromFiles({
		{ "core/graphic/shaderSrc/deferredRendering/lightingPassShader/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/deferredRendering/lightingPassShader/fs.glsl", ShaderType::Fragment }
	});
	std::initializer_list<FrameBufferSpecification> specs =
	{
		{
			AttachmentType::Color,
			TextureInternalFormat::RGBA16F,
			TextureDataFormat::RGBA,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureFilter::NEAREST,
			TextureFilter::NEAREST
		}
	};
	mFrameBuffer = std::make_shared<FrameBuffer>(glm::vec3{ mSize.x, mSize.y ,0 }, specs);
}

void LightingPass::runPass(Scene* scene)
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

void LightingPass::beginPass()
{
	//binding program first
	RenderPass::beginPass();
	//get colorattachment from gpass
	//set uniform textureSampler2D
 	auto geometryBuffer = mPrevPass->getCurrentFrameBuffer();
	auto gPos = geometryBuffer->getColorAttachment(0);
	auto gNormal = geometryBuffer->getColorAttachment(1);
	auto gDiff = geometryBuffer->getColorAttachment(2);
	auto gSpecShiness = geometryBuffer->getColorAttachment(3);
	auto gMaterialType = geometryBuffer->getColorAttachment(4);
	gPos->bind(0);
	gNormal->bind(1);
	gDiff->bind(2);
	gSpecShiness->bind(3);
	gMaterialType->bind(4);

	mProgram->setUniform("gPosition", 0);
	mProgram->setUniform("gNormal", 1);
	mProgram->setUniform("gDiffuse", 2);
	mProgram->setUniform("gSpecShiness", 3);
	mProgram->setUniform("gMaterialType", 4);

	FrameBuffer* ssaoBuffer = mOwner->getFrameBuffer(RenderPassKey::SSAO);
	mProgram->setUniform("enableSSAO", ssaoBuffer != nullptr);
	if (ssaoBuffer)
	{
		auto ssaoTex = ssaoBuffer->getColorAttachment(0);
		ssaoTex->bind(5);
		mProgram->setUniform("ssaoMap", 5);
	}

	auto csmPass = mOwner->getRenderPass(RenderPassKey::CSM);
	mProgram->setUniform("enableCSM", csmPass != nullptr);
	if (csmPass)
	{
		FrameBuffer* csmBuffer = csmPass->getCurrentFrameBuffer();
		Texture* shadowDepthTex = csmBuffer->getDepthStencilAttachment();
		shadowDepthTex->bind(6);
		mProgram->setUniform("cascadedShadowMap", 6);
		CascadeShadowMapPass* cascadedShadowMapPass = static_cast<CascadeShadowMapPass*>(csmPass);
		
		//set cascadedLayernum 
		mProgram->setUniform("cascadedLayerCount", cascadedShadowMapPass->getCascadedLayerCount());
		//set cascadedDistances
		auto csmDistances = cascadedShadowMapPass->getCascadedFrustumDistanes();
		mProgram->setUniform("cascadedLayerDistances", csmDistances.size(), csmDistances.data());

		//set cascaded lightIndex
		mProgram->setUniform("cascadedShadowLightIndex", cascadedShadowMapPass->getShadowLightIndex());

		//pcfSize
		mProgram->setUniform("pcfSize", cascadedShadowMapPass->getPcfSize());

		//set display cascadedColor
		mProgram->setUniform("displayCacadedColor", cascadedShadowMapPass->getDisplayCacadedColor());

		//pcf enable
		mProgram->setUniform("enablePCF", cascadedShadowMapPass->getEnablePCF());
	}
}