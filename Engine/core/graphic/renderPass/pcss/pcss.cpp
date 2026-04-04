#include "graphic/renderPass/bloomPass/bloomPass.h"
#include "graphic/texture/texture2D/texture2D.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "graphic/renderer/renderer.h"
#include "graphic/program/program.h"
#include "geometry/screenQuad.h"
#include "pcss.h"
#include "scene/scene.h"
#include "core.h"
#include "scene/renderObject.h"
#include "scene/light/light.h"
#include "gtc/type_ptr.hpp"

PCSS::PCSS(Renderer* r, const RenderState& state)
	:RenderPass(r, state)
{
	//lightMatrices uniform buffer bindingPoint  = 3
	//viewprojectionMatrix + viewMatrix;
	mLightMatricesBuffer = std::make_unique<UniformBuffer>(2 * sizeof(glm::mat4), 3);
	//texture array depthBuffer fbo
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
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureFilter::NEAREST,
			TextureFilter::NEAREST,
			{1.0, 1.0, 1.0, 1.0}
		}
	};
	mFrameBuffer = std::make_shared<FrameBuffer>(glm::vec3{ mSize.x, mSize.y , 0 }, fboSpec);


	mProgram = std::make_shared<Program>();
	std::initializer_list<ShaderFile> shaders =
	{
		{ "core/graphic/shaderSrc/pcss/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/pcss/fs.glsl", ShaderType::Fragment }
	};
	mProgram->buildFromFiles(shaders);
	updateLightViewMatrix();
}

PCSS::~PCSS()
{
}

void PCSS::beginPass()
{
	RenderPass::beginPass();
	mLightMatricesBuffer->bind();
}

void PCSS::runPass(Scene* scene)
{
	for (auto& renderObject : scene->getRenderList())
	{
		if (renderObject->getType() != RenderObjectType::Mesh)
		{
			continue;
		}
		renderObject->beginDraw(mProgram.get());
		renderObject->draw();
		renderObject->endDraw(mProgram.get());
	}
}

int PCSS::getBlockSearchSamples() const
{
	return mBlockerSearchSamples;
}

float PCSS::getLightRadiusUV() const
{
	return mLightRadiusUV;
}

float PCSS::getLightNear() const
{
	return mLightNear;
}

float PCSS::getLightFar() const
{
	return mLightFar;
}

int PCSS::getPcfSamples() const
{
	return mPcfSamples;
}

void PCSS::setBlockSearchSamples(int sample)
{
	mBlockerSearchSamples = sample;
}

void PCSS::setLightRadiusUV(float lightUVSize)
{
	mLightRadiusUV = lightUVSize;
}

void PCSS::setLightNear(float lightNear)
{
	mLightNear = lightNear;
}

void PCSS::setLightFar(float lightFar)
{
	mLightFar = lightFar;
}

void PCSS::setPcfSamples(int sample)
{
	mPcfSamples = sample;
}

void PCSS::updateLightViewMatrix()
{
	int lightIndex = mOwner->getCurrentScene()->getShadowLightIndex();
	if (lightIndex == -1)
	{
		//TODO
		unsigned int clearColor = 0;
		mLightMatricesBuffer->clearBuffer(GL_R8UI, sizeof(glm::mat4) * 2, &clearColor, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 0);
		return;
	}
	Light* light = mOwner->getCurrentScene()->getLights().at(lightIndex).get();
	
	glm::vec3 lightPos = light->getPosition();
	glm::vec3 lightDirection = light->getDirection();
	//TODO:: 
	glm::mat4 lightViewSpace = glm::lookAt(lightPos, {0, 0, 0}, { 0, 1, 0 });
	float size = 5.0f;
	mLightNear = 1.0f;
	mLightFar = 100.0f;
	glm::mat4 lightProjection = glm::ortho(-size, size, -size, size, mLightNear, mLightFar);
	
	mLightMatricesBuffer->setData(sizeof(glm::mat4), glm::value_ptr(lightProjection * lightViewSpace), 0);
	mLightMatricesBuffer->setData(sizeof(glm::mat4), glm::value_ptr(lightViewSpace), sizeof(glm::mat4));
}

Texture* PCSS::getDebugView() const
{
	return mFrameBuffer->getDepthStencilAttachment();
}
