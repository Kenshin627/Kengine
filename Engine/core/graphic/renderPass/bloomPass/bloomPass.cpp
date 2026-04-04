#include "core.h"
#include "BloomPass.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include <graphic/program/program.h>
#include "geometry/screenQuad.h"
#include "scene/scene.h"
#include "graphic/texture/texture.h"
#include "graphic/texture/texture2D/texture2D.h"

static int downSamples = 7;
static int downSize = 2;

BloomPass::BloomPass(Renderer* r, const RenderState& state)
    :RenderPass(r, state)
{
    mDownSampleProgram = std::make_unique<Program>();
    mUpSampleProgram = std::make_unique<Program>();
    mExtractHighLightProgram = std::make_unique<Program>();

    mDownSampleProgram->buildFromFiles({
        { "core/graphic/shaderSrc/bloom/downsample/downSampleComp.glsl", ShaderType::Compute }
    });

    mUpSampleProgram->buildFromFiles({
        { "core/graphic/shaderSrc/bloom/upsample/upSampleComp.glsl", ShaderType::Compute }
    });

    mExtractHighLightProgram->buildFromFiles(
        {
            { "core/graphic/shaderSrc/bloom/extractHighlight/vs.glsl", ShaderType::Vertex },
            { "core/graphic/shaderSrc/bloom/extractHighlight/fs.glsl", ShaderType::Fragment }
    });

    mDownSampleFBOs.reserve(downSamples);
    mUpSampleFBOs.reserve(downSamples);
    int sourceWidth = state.viewport.z;
    int sourceHeight = state.viewport.w;

    std::initializer_list<FrameBufferSpecification> extractHighLightSpecs =
    {
        //color attachmeng 1 hdr
        {
            AttachmentType::Color,
            TextureInternalFormat::RGBA16F,
            TextureDataFormat::RGBA,
            TextureWarpMode::CLAMP_TO_EDGE,
            TextureWarpMode::CLAMP_TO_EDGE,
            TextureFilter::LINEAR,
            TextureFilter::LINEAR
        },
        //color attachment 0 normal ldr 
        {
            AttachmentType::Color,
            TextureInternalFormat::RGBA16F,
            TextureDataFormat::RGBA,
            TextureWarpMode::CLAMP_TO_EDGE,
            TextureWarpMode::CLAMP_TO_EDGE,
            TextureFilter::LINEAR,
            TextureFilter::LINEAR
        }
    };
    mExtractHighLightFBO = std::make_unique<FrameBuffer>(glm::vec3{ mSize.x, mSize.y, 0 }, extractHighLightSpecs);

    buildDownUpSampleTexture(sourceWidth / 2, sourceHeight / 2);
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
    mExtractHighLightFBO->bind();
    mExtractHighLightProgram->bind();
    updateRenderState();
    Texture* tex = mPrevPass->getCurrentFrameBuffer()->getColorAttachment(0);
    tex->bind();
    mExtractHighLightProgram->setUniform("screenMap", 0);
    mExtractHighLightProgram->setUniform("thresholdMin", mThresholdMin);
    mExtractHighLightProgram->setUniform("thresholdMax", mThresholdMax);
    ScreenQuad* quad = scene->getScreenQuad();
    if (!quad)
    {
        KS_CORE_ERROR("pass screen quad is null");
        return;
    }
    quad->beginDraw();
    quad->draw();

    //DownSampleChains
    mDownSampleProgram->bind();
    mDownSampleProgram->setUniform("uDownSampleBlurSize", mBlurRadius);
    mDownSampleProgram->setUniform("uDownSampleBlurSigma", mGaussianSigma);
    mDownSampleProgram->setUniform("uBloomIntensity", mBloomIntensity);
    mDownSampleProgram->setUniform("uFirstDownSample", 1);

    mDownSampleTexture->bindImage2D(1, 0, GL_WRITE_ONLY);
    auto source = mExtractHighLightFBO->getColorAttachment(0);
    mDownSampleProgram->setUniform("uSourceTex", 0);
    source->bind(0);
    mDownSampleProgram->setUniform("uSourceMipmap", 0);

    glDispatchCompute((mRenderState.viewport.z / 2 + 7) / 8, (mRenderState.viewport.w / 2 + 7) / 8, 1.0);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    mDownSampleTexture->bind(0);
    for (int i = 1; i < downSamples; ++i)
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
	mUpSampleProgram->setUniform("uPrevMipLevel", downSamples - 1);
    mUpSampleProgram->setUniform("uCurrentMipLevel", downSamples - 2);
	mDownSampleTexture->bind(1);
    mUpSampleTexture->bindImage2D(2, downSamples - 2, GL_WRITE_ONLY);
    glDispatchCompute((mDownSampleTexture->width() / (1 << (downSamples - 2)) + 7) / 8, (mDownSampleTexture->height() / (1 << (downSamples - 2)) + 7) / 8, 1.0);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    mUpSampleTexture->bind(0);

    for (int i = 1; i < downSamples - 1; ++i)
    {
        mUpSampleProgram->setUniform("uPrevMipLevel", downSamples - 1 - i);
        mUpSampleProgram->setUniform("uCurrentMipLevel", downSamples - 2 - i);
        mUpSampleTexture->bindImage2D(2, downSamples - 2 - i, GL_WRITE_ONLY);
        glDispatchCompute((mUpSampleTexture->width() / (1 << (downSamples - 2 - i)) + 7) / 8, (mDownSampleTexture->height() / (1 << (downSamples - 2 - i)) + 7) / 8, 1.0);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
}

void BloomPass::endPass()
{
}

void BloomPass::resize(uint width, uint height)
{
    mRenderState.viewport.z = width;
    mRenderState.viewport.w = height;
    buildDownUpSampleTexture(width / 2, height / 2);
    mExtractHighLightFBO->resize(width, height);
}

Texture* BloomPass::getHDRTexture() const
{
    return mUpSampleTexture.get();
}

Texture* BloomPass::getLDRTexture() const
{
    return mExtractHighLightFBO->getColorAttachment(1);
}

FrameBuffer* BloomPass::getDebugView() const
{
    return mUpSampleFBOs[downSamples - 2].get();
}

void BloomPass::buildDownUpSampleTexture(uint width, uint height)
{
    TextureSpecification mipChainSpec;
    mipChainSpec.dataFormat = TextureDataFormat::RGBA;
    mipChainSpec.width = width;
    mipChainSpec.height = height;
    mipChainSpec.internalFormat = TextureInternalFormat::RGBA16F;
    mipChainSpec.mipmapLevel = downSamples;
    mipChainSpec.minFilter = TextureFilter::LINEAR_MIPMAP_LINEAR;
    mipChainSpec.magFilter = TextureFilter::LINEAR;
    mipChainSpec.warpS = TextureWarpMode::CLAMP_TO_EDGE;
    mipChainSpec.warpT = TextureWarpMode::CLAMP_TO_EDGE;
    mDownSampleTexture = std::make_unique<Texture2D>(mipChainSpec);
    glTextureStorage2D(mDownSampleTexture->id(), mipChainSpec.mipmapLevel, Texture::convertToGLInternalFormat(mipChainSpec.internalFormat), width, height);
    mUpSampleTexture = std::make_unique<Texture2D>(mipChainSpec);
    glTextureStorage2D(mUpSampleTexture->id(), mipChainSpec.mipmapLevel - 1, Texture::convertToGLInternalFormat(mipChainSpec.internalFormat), width, height);
}
