#include "core.h"
#include "BloomPass.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include <graphic/program/program.h>
#include "geometry/screenQuad.h"
#include "scene/scene.h"
#include "graphic/texture/texture2D/texture2D.h"

BloomPass::BloomPass(Renderer* r, const RenderState& state)
    :RenderPass(r, state)
{
    mExtractHighLightProgram = std::make_unique<Program>();
    mDownSampleProgram = std::make_unique<Program>();
    mUpSampleProgram = std::make_unique<Program>();

    mExtractHighLightProgram->buildFromFiles(
    {
        { "core/graphic/shaderSrc/bloom/extractComp.glsl", ShaderType::Compute }
    });

    mDownSampleProgram->buildFromFiles({
        { "core/graphic/shaderSrc/bloom/downSampleComp.glsl", ShaderType::Compute }
    });

    mUpSampleProgram->buildFromFiles({
        { "core/graphic/shaderSrc/bloom/upSampleComp.glsl", ShaderType::Compute }
    });
    
    buildDownUpSampleTexture(state.viewport.z, state.viewport.w);
}

BloomPass::~BloomPass()
{
    //nothing to do
}

void BloomPass::beginPass()
{
    //noting to do
}

void BloomPass::runPass(Scene* scene)
{
    //Extract highLights
    mExtractHighLightProgram->bind();
    Texture* tex = mPrevPass->getCurrentFrameBuffer()->getColorAttachment(0);
    tex->bindImage2D(0, 0, GL_READ_ONLY);
	mLDRTexture->bindImage2D(1, 0, GL_WRITE_ONLY);
	mHDRTexture->bindImage2D(2, 0, GL_WRITE_ONLY);
    mExtractHighLightProgram->setUniform("thresholdMin", mThresholdMin);
    mExtractHighLightProgram->setUniform("thresholdMax", mThresholdMax);
    glDispatchCompute((mRenderState.viewport.z + 7) / 8, (mRenderState.viewport.w + 7) / 8, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    //DownSampleChains
    mDownSampleProgram->bind();
    mDownSampleProgram->setUniform("uDownSampleBlurSize", mBlurRadius);
    mDownSampleProgram->setUniform("uDownSampleBlurSigma", mGaussianSigma);
    mDownSampleProgram->setUniform("uBloomIntensity", mBloomIntensity);
    mDownSampleProgram->setUniform("uFirstDownSample", 1);
    mDownSampleTexture->bindImage2D(1, 0, GL_WRITE_ONLY);   
    mDownSampleProgram->setUniform("uSourceTex", 0);
    mHDRTexture->bind(0);
    mDownSampleProgram->setUniform("uSourceMipmap", 0);
    glDispatchCompute((mRenderState.viewport.z / 2 + 7) / 8, (mRenderState.viewport.w / 2 + 7) / 8, 1.0);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    mDownSampleTexture->bind(0);
    for (int i = 1; i < mMipmapLevels; ++i)
    {
        mDownSampleProgram->setUniform("uSourceMipmap", i - 1);
        mDownSampleTexture->bindImage2D(1, i, GL_WRITE_ONLY);
        mDownSampleProgram->setUniform("uFirstDownSample", 0);
        uint width = mDownSampleTexture->width() / (1 << i);
        uint height = mDownSampleTexture->height() / (1 << i);
        glDispatchCompute((width + 7) / 8, (height + 7) / 8, 1.0);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    //UpSampleChains
    mUpSampleProgram->bind();
    mUpSampleProgram->setUniform("uUpSampleBlurSize", mBlurRadius);
    mUpSampleProgram->setUniform("uUpSampleBlurSigma", mGaussianSigma);
    mUpSampleProgram->setUniform("uBloomIntensity", mBloomIntensity);
    mUpSampleProgram->setUniform("uPrevTex", 0);
    mUpSampleProgram->setUniform("uCurrentTex", 1);
	mUpSampleProgram->setUniform("uPrevMipLevel", mMipmapLevels - 1);
    mUpSampleProgram->setUniform("uCurrentMipLevel", mMipmapLevels - 2);
	mDownSampleTexture->bind(1);
    mUpSampleTexture->bindImage2D(2, mMipmapLevels - 2, GL_WRITE_ONLY);
    glDispatchCompute((mDownSampleTexture->width() / (1 << (mMipmapLevels - 2)) + 7) / 8, (mDownSampleTexture->height() / (1 << (mMipmapLevels - 2)) + 7) / 8, 1.0);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    mUpSampleTexture->bind(0);

    for (int i = 1; i < mMipmapLevels - 1; ++i)
    {
        mUpSampleProgram->setUniform("uPrevMipLevel", mMipmapLevels - 1 - i);
        mUpSampleProgram->setUniform("uCurrentMipLevel", mMipmapLevels - 2 - i);
        mUpSampleTexture->bindImage2D(2, mMipmapLevels - 2 - i, GL_WRITE_ONLY);
        glDispatchCompute((mUpSampleTexture->width() / (1 << (mMipmapLevels - 2 - i)) + 7) / 8, (mDownSampleTexture->height() / (1 << (mMipmapLevels - 2 - i)) + 7) / 8, 1.0);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
}

void BloomPass::endPass()
{
	//nothing to do
}

void BloomPass::resize(uint width, uint height)
{
    mRenderState.viewport.z = width;
    mRenderState.viewport.w = height;
    buildDownUpSampleTexture(width, height);
}

Texture* BloomPass::getHDRTexture() const
{
    return mUpSampleTexture.get();
}

Texture* BloomPass::getLDRTexture() const
{
    return mLDRTexture.get();
}

Texture* BloomPass::getDebugView() const
{
	return mUpSampleTexture.get();
}

void BloomPass::buildDownUpSampleTexture(uint width, uint height)
{
    TextureSpecification mipChainSpec;
    mipChainSpec.dataFormat = TextureDataFormat::RGBA;
    mipChainSpec.width = width / 2;
    mipChainSpec.height = height / 2;
    mipChainSpec.internalFormat = TextureInternalFormat::RGBA16F;
    mipChainSpec.mipmapLevel = mMipmapLevels;
    mipChainSpec.minFilter = TextureFilter::LINEAR_MIPMAP_LINEAR;
    mipChainSpec.magFilter = TextureFilter::LINEAR;
    mipChainSpec.warpS = TextureWarpMode::CLAMP_TO_EDGE;
    mipChainSpec.warpT = TextureWarpMode::CLAMP_TO_EDGE;
    mDownSampleTexture = std::make_unique<Texture2D>(mipChainSpec);
    glTextureStorage2D(mDownSampleTexture->id(), mipChainSpec.mipmapLevel, Texture::convertToGLInternalFormat(mipChainSpec.internalFormat), width / 2, height / 2);
    mUpSampleTexture = std::make_unique<Texture2D>(mipChainSpec);
    glTextureStorage2D(mUpSampleTexture->id(), mipChainSpec.mipmapLevel - 1, Texture::convertToGLInternalFormat(mipChainSpec.internalFormat), width / 2, height / 2);

    TextureSpecification extractHighLightSpec;
    extractHighLightSpec.dataFormat = TextureDataFormat::RGBA;
    extractHighLightSpec.width = width;
    extractHighLightSpec.height = height;
    extractHighLightSpec.internalFormat = TextureInternalFormat::RGBA16F;
    extractHighLightSpec.mipmapLevel = 1;
    extractHighLightSpec.minFilter = TextureFilter::LINEAR;
    extractHighLightSpec.magFilter = TextureFilter::LINEAR;
    extractHighLightSpec.warpS = TextureWarpMode::CLAMP_TO_EDGE;
    extractHighLightSpec.warpT = TextureWarpMode::CLAMP_TO_EDGE;
    mLDRTexture = std::make_unique<Texture2D>(extractHighLightSpec);
    glTextureStorage2D(mLDRTexture->id(), 1, Texture::convertToGLInternalFormat(extractHighLightSpec.internalFormat), width, height);
    mHDRTexture = std::make_unique<Texture2D>(extractHighLightSpec);
    glTextureStorage2D(mHDRTexture->id(), 1, Texture::convertToGLInternalFormat(extractHighLightSpec.internalFormat), width, height);
}
