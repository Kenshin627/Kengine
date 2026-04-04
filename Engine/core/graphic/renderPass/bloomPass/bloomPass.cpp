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
        //{ "core/graphic/shaderSrc/bloom/quad.glsl", ShaderType::Vertex },
        //{ "core/graphic/shaderSrc/bloom/downsample/downSample.glsl", ShaderType::Fragment }
        { "core/graphic/shaderSrc/bloom/downsample/downSampleComp.glsl", ShaderType::Compute }
        });

    mUpSampleProgram->buildFromFiles({
        //{ "core/graphic/shaderSrc/bloom/quad.glsl", ShaderType::Vertex },
        //{ "core/graphic/shaderSrc/bloom/upsample/upSample.glsl", ShaderType::Fragment }
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

    std::initializer_list<FrameBufferSpecification> specs =
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
        }
    };

    // create downSampleFBOs
    for (int i = 0; i < downSamples; ++i)
    {
        int w = sourceWidth / downSize;
        int h = sourceHeight / downSize;
        mDownSampleFBOs.push_back(std::make_unique<FrameBuffer>(glm::vec3{ w, h, 0 }, specs));
        downSize *= 2;
    }

    // createUpSampleFbos
    for (int i = 0; i < downSamples - 1; ++i)
    {
        int w = mDownSampleFBOs[downSamples - 2 - i]->getColorAttachment(0)->width();
        int h = mDownSampleFBOs[downSamples - 2 - i]->getColorAttachment(0)->height();
        mUpSampleFBOs.push_back(std::make_unique<FrameBuffer>(glm::vec3{ w, h, 0 }, specs));
    }

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
    //source->bindImage2D(0, 0, GL_READ_ONLY);
    mDownSampleProgram->setUniform("uSourceTex", 0);
    source->bind(0);
    mDownSampleProgram->setUniform("uSourceMipmap", 0);

    glDispatchCompute((mRenderState.viewport.z / 2 + 7) / 8, (mRenderState.viewport.w / 2 + 7) / 8, 1.0);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    mDownSampleTexture->bind(0);
    for (int i = 1; i < downSamples; ++i)
    {
        mDownSampleProgram->setUniform("uSourceMipmap", i - 1);
        //mDownSampleTexture->bindImage2D(0, i - 1, GL_READ_ONLY);
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

    //layout(binding = 0, rgba16f) uniform image2D uPrevTex;
    //layout(binding = 1, rgba16f) uniform image2D uCurrentTex;
    //layout(binding = 2, rgba16f) uniform image2D uDestTex;
    mUpSampleProgram->setUniform("uPrevTex", 0);
    mUpSampleProgram->setUniform("uCurrentTex", 1);
	mUpSampleProgram->setUniform("uPrevMipLevel", downSamples - 1);
    mUpSampleProgram->setUniform("uCurrentMipLevel", downSamples - 2);
	mDownSampleTexture->bind(1);
    //mDownSampleTexture->bindImage2D(0, downSamples - 1, GL_READ_ONLY);
    //mDownSampleTexture->bindImage2D(1, downSamples - 2, GL_READ_ONLY);
    mUpSampleTexture->bindImage2D(2, downSamples - 2, GL_WRITE_ONLY);
    glDispatchCompute((mDownSampleTexture->width() / (1 << (downSamples - 2)) + 7) / 8, (mDownSampleTexture->height() / (1 << (downSamples - 2)) + 7) / 8, 1.0);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    mUpSampleTexture->bind(0);
    //for (int i = 1; i < downSamples - 1; ++i)
    //{
        for (int i = 1; i < downSamples - 1; ++i)
        {
            //mUpSampleTexture->bindImage2D(0, downSamples - 1 - i, GL_READ_ONLY);
            //mDownSampleTexture->bindImage2D(1, downSamples - 2 - i, GL_READ_ONLY);
            mUpSampleProgram->setUniform("uPrevMipLevel", downSamples - 1 - i);
            mUpSampleProgram->setUniform("uCurrentMipLevel", downSamples - 2 - i);
            mUpSampleTexture->bindImage2D(2, downSamples - 2 - i, GL_WRITE_ONLY);
            glDispatchCompute((mUpSampleTexture->width() / (1 << (downSamples - 2 - i)) + 7) / 8, (mDownSampleTexture->height() / (1 << (downSamples - 2 - i)) + 7) / 8, 1.0);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
    //}
}

void BloomPass::endPass()
{
}

void BloomPass::resize(uint width, uint height)
{
    //resize viewport
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
    TextureSpecification downSampleSpec;
    downSampleSpec.dataFormat = TextureDataFormat::RGBA;
    downSampleSpec.width = width;
    downSampleSpec.height = height;
    downSampleSpec.internalFormat = TextureInternalFormat::RGBA16F;
    downSampleSpec.mipmapLevel = downSamples;
    downSampleSpec.minFilter = TextureFilter::LINEAR_MIPMAP_LINEAR;
    downSampleSpec.magFilter = TextureFilter::LINEAR;
    downSampleSpec.warpS = TextureWarpMode::CLAMP_TO_EDGE;
    downSampleSpec.warpT = TextureWarpMode::CLAMP_TO_EDGE;
    mDownSampleTexture = std::make_unique<Texture2D>(downSampleSpec);
    glTextureStorage2D(mDownSampleTexture->id(), downSampleSpec.mipmapLevel, Texture::convertToGLInternalFormat(downSampleSpec.internalFormat), width, height);

    TextureSpecification upSampleSpec;
    upSampleSpec.dataFormat = TextureDataFormat::RGBA;
    upSampleSpec.width = width;
    upSampleSpec.height = height;
    upSampleSpec.internalFormat = TextureInternalFormat::RGBA16F;
    upSampleSpec.mipmapLevel = downSamples - 1;
    upSampleSpec.minFilter = TextureFilter::LINEAR_MIPMAP_LINEAR;
    upSampleSpec.magFilter = TextureFilter::LINEAR;
    upSampleSpec.warpS = TextureWarpMode::CLAMP_TO_EDGE;
    upSampleSpec.warpT = TextureWarpMode::CLAMP_TO_EDGE;
    mUpSampleTexture = std::make_unique<Texture2D>(upSampleSpec);
    glTextureStorage2D(mUpSampleTexture->id(), upSampleSpec.mipmapLevel, Texture::convertToGLInternalFormat(upSampleSpec.internalFormat), width, height);
}
