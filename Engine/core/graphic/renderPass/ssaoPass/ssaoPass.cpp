#include <random>
#include "core.h"
#include "scene/scene.h"
#include "ssaoPass.h"
#include "graphic/program/program.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "geometry/screenQuad.h"
#include "graphic/texture/texture2D/texture2D.h"
#include "graphic/renderer/renderer.h"
#include "../deferredRendering/geometryPass.h"

SSAOPass::SSAOPass(const SSAOSpecification& spec, Renderer* r, const RenderState& state)
	:RenderPass(r, state),
	 mSpec(spec)
{
	//buildSampler
	buildSamplers();
	//buildNoiseTexture
	buildNoiseTexture();
	//buildProgram
	mProgram = std::make_shared<Program>();
	std::initializer_list<ShaderFile> shaders =
	{
		{ "core/graphic/shaderSrc/ssao/occlusion/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/ssao/occlusion/fs.glsl", ShaderType::Fragment }
	};
	mProgram->buildFromFiles(shaders);
	//buildFrameBuffer
	//SSAO BUFFER
	//|		RGB8  | occlusion		| 
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
	mFrameBuffer = std::make_unique<FrameBuffer>(glm::vec3{ mSize.x, mSize.y ,0 }, specs);
}

void SSAOPass::setKernelSize(uint kernelSize)
{
	if (mSpec.kernelSize != kernelSize)
	{
		mSpec.kernelSize = kernelSize;
		buildSamplers();
	}
}

uint SSAOPass::getKernelSize() const
{
	return mSpec.kernelSize;
}

void SSAOPass::setSamplerRadius(float radius)
{
	mSpec.samplerRadius = radius;
}

float SSAOPass::getSamplerRadius() const
{
	return mSpec.samplerRadius;
}

void SSAOPass::setBias(float bias)
{
	mSpec.bias = bias;
}

float SSAOPass::getBias() const
{
	return mSpec.bias;
}

void SSAOPass::beginPass()
{
	RenderPass::beginPass();
	//send kernelSampler array to GPU use uniform
	for (GLuint i = 0; i < mSpec.kernelSize; ++i)
	{
		glUniform3fv(glGetUniformLocation(mProgram->id(), ("samplers[" + std::to_string(i) + "]").c_str()), 1, &mKernelSamplers[i][0]);
	}
	//send radius to GPU use uniform
	mProgram->setUniform("samperRadius", mSpec.samplerRadius);
	//send kernelSize to GPU use uniform
	mProgram->setUniform("kernelSize", mSpec.kernelSize);
	mProgram->setUniform("bias", mSpec.bias);
	//upload gPositionTexture + gNormalTexture
	//TODO
	GeometryPass* gPass = static_cast<GeometryPass*>(mOwner->getRenderPass(RenderPassKey::GEOMETRY));
	Texture* gPosDepth = gPass->getCurrentFrameBuffer()->getColorAttachment(0);
	Texture* gNormal   = gPass->getCurrentFrameBuffer()->getColorAttachment(1);
	gPosDepth->bind(0);
	gNormal->bind(1);
	mProgram->setUniform("gPosition", 0);
	mProgram->setUniform("gNormal", 1);
	//upload noiseTexture
	mNoiseTexture->bind(2);
	mProgram->setUniform("noise", 2);
	//update framebuffersize
	mProgram->setUniform("frameBufferSize", mSize);
	//upload projectMaTrix in cameraBuffer
	
}

void SSAOPass::runPass(Scene* scene)
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

void SSAOPass::buildSamplers()
{
	std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
	std::default_random_engine engine;
	mKernelSamplers.clear();
	mKernelSamplers.reserve(mSpec.kernelSize);
	for (size_t i = 0; i < mSpec.kernelSize; i++)
	{
		glm::vec4 sampler
		{
			randomFloats(engine) * 2.0f - 1.0f,
			randomFloats(engine) * 2.0f - 1.0f,
			randomFloats(engine),
			0.0f
		};
		sampler = glm::normalize(sampler);
		sampler *= randomFloats(engine);
		float scale = i / static_cast<float>(mSpec.kernelSize);
		scale = lerp(0.1f, 1.0f, scale * scale);
		sampler *= scale;
		mKernelSamplers.push_back(sampler);
	}
}

void SSAOPass::buildNoiseTexture()
{
	//4x4 noiseTexture
	//tbn rotate around zAxis
	std::uniform_real_distribution<float> randomFloat(0.0f, 1.0f);
	std::default_random_engine  engine;
	std::vector<glm::vec3> noiseData;
	noiseData.reserve(16);
	for (size_t i = 0; i < 4; i++)
	{
		for(size_t j = 0; j < 4; j++)
		{
			noiseData.emplace_back(randomFloat(engine) * 2.0f - 1.0f, randomFloat(engine) * 2.0f - 1.0f, 0.0f);
		}
	}
	TextureSpecification textureSpec;
	textureSpec.width = 4;
	textureSpec.height = 4;
	textureSpec.warpS = TextureWarpMode::REPEAT;
	textureSpec.warpT = TextureWarpMode::REPEAT;
	textureSpec.minFilter = TextureFilter::NEAREST;
	textureSpec.magFilter = TextureFilter::NEAREST;
	textureSpec.mipmapLevel = 1;
	textureSpec.internalFormat = TextureInternalFormat::RGB32F;
	textureSpec.dataFormat = TextureDataFormat::RGB;
	textureSpec.chanel = 3;
	mNoiseTexture = std::make_unique<Texture2D>(textureSpec);
	mNoiseTexture->loadFromData(4, 4, noiseData.data(), 3, TextureInternalFormat::RGB32F, TextureDataFormat::RGB, GL_FLOAT);
}
