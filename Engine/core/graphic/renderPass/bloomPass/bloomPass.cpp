#include "core.h"
#include "BloomPass.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include <graphic/program/program.h>
#include "geometry/screenQuad.h"
#include "scene/scene.h"
#include "graphic/texture/texture.h"

static int downSamples = 7;
static int downSize = 2;

BloomPass::BloomPass(Renderer* r, const RenderState& state)
    :RenderPass(r, state)
{
    mDownSampleProgram = std::make_unique<Program>();
    mUpSampleProgram = std::make_unique<Program>();
    mExtractHighLightProgram = std::make_unique<Program>();

    mDownSampleProgram->buildFromFiles({
        { "core/graphic/shaderSrc/bloom2/quad.glsl", ShaderType::Vertex },
        { "core/graphic/shaderSrc/bloom2/downSample.glsl", ShaderType::Fragment }
    });

    mUpSampleProgram->buildFromFiles({
        { "core/graphic/shaderSrc/bloom2/quad.glsl", ShaderType::Vertex },
        { "core/graphic/shaderSrc/bloom2/upSample.glsl", ShaderType::Fragment }
    });

    mExtractHighLightProgram->buildFromFiles(
    {
        { "core/graphic/shaderSrc/bloom/vs.glsl", ShaderType::Vertex },
        { "core/graphic/shaderSrc/bloom/fs.glsl", ShaderType::Fragment }
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
}

BloomPass::~BloomPass()
{
}

void BloomPass::beginPass()
{
    mExtractHighLightFBO->bind();
    mExtractHighLightProgram->bind();
    updateRenderState();
    Texture* tex = mPrevPass->getCurrentFrameBuffer()->getColorAttachment(0);
    tex->bind();
    mExtractHighLightProgram->setUniform("screenMap", 0);
    mExtractHighLightProgram->setUniform("thresholdMin", mThresholdMin);
    mExtractHighLightProgram->setUniform("thresholdMax", mThresholdMax);
}

void BloomPass::runPass(Scene* scene)
{
    ScreenQuad* quad = scene->getScreenQuad();
    if (!quad)
    {
        KS_CORE_ERROR("pass screen quad is null");
        return;
    }
    quad->beginDraw();
    quad->draw();

    // downSampleChains
    auto source = mExtractHighLightFBO->getColorAttachment(0);
    mDownSampleProgram->bind();
    mDownSampleFBOs[0]->bind();
    source->bind();
    mDownSampleProgram->setUniform("uSourceTex", 0);
    mDownSampleProgram->setUniform("uDownSampleBlurSize", mBlurRadius);
    mDownSampleProgram->setUniform("uDownSampleBlurSigma", mGaussianSigma);
    mDownSampleProgram->setUniform("uBloomIntensity", mBloomIntensity);
    mDownSampleProgram->setUniform("uFirstDownSample", 1);
    glViewport(0, 0, mDownSampleFBOs[0]->getColorAttachment(0)->width(), mDownSampleFBOs[0]->getColorAttachment(0)->height());
    glClear(mRenderState.clearBits);
    glClearColor(mRenderState.clearColor.r, mRenderState.clearColor.g, mRenderState.clearColor.b, mRenderState.clearColor.a);
    quad->draw();

    for (int i = 1; i < downSamples; ++i)
    {
        mDownSampleFBOs[i - 1]->getColorAttachment(0)->bind();
        mDownSampleFBOs[i]->bind();
        mDownSampleProgram->setUniform("uFirstDownSample", 0);

        glViewport(0, 0, mDownSampleFBOs[i]->getColorAttachment(0)->width(), mDownSampleFBOs[i]->getColorAttachment(0)->height());
        glClear(mRenderState.clearBits);
        glClearColor(mRenderState.clearColor.r, mRenderState.clearColor.g, mRenderState.clearColor.b, mRenderState.clearColor.a);
        quad->draw();
    }

    // upSampleChains
    mUpSampleProgram->bind();
    mUpSampleProgram->setUniform("uPrevTex", 0);
    mUpSampleProgram->setUniform("uCurrentTex", 1);
    mUpSampleProgram->setUniform("uUpSampleBlurSize", mBlurRadius);
    mUpSampleProgram->setUniform("uUpSampleBlurSigma", mGaussianSigma);
    mUpSampleProgram->setUniform("uBloomIntensity", mBloomIntensity);

    mDownSampleFBOs[downSamples - 1]->getColorAttachment(0)->bind(0);  // 最小downSample → uPrevTex
    mDownSampleFBOs[downSamples - 2]->getColorAttachment(0)->bind(1);  // 次小downSample → uCurrentTex
    mUpSampleFBOs[0]->bind();

    glViewport(0, 0, mUpSampleFBOs[0]->getColorAttachment(0)->width(), mUpSampleFBOs[0]->getColorAttachment(0)->height());
    glClear(mRenderState.clearBits);
    glClearColor(mRenderState.clearColor.r, mRenderState.clearColor.g, mRenderState.clearColor.b, mRenderState.clearColor.a);
    quad->draw();

    for (int i = 1; i < downSamples - 1; ++i)
    {

        for (int i = 1; i < downSamples - 1; ++i)
        {
            mUpSampleFBOs[i - 1]->getColorAttachment(0)->bind(0);
            mDownSampleFBOs[downSamples - 2 - i]->getColorAttachment(0)->bind(1);
            mUpSampleFBOs[i]->bind();

            glViewport(0, 0, mUpSampleFBOs[i]->getColorAttachment(0)->width(), mUpSampleFBOs[i]->getColorAttachment(0)->height());
            glClear(mRenderState.clearBits);
            glClearColor(mRenderState.clearColor.r, mRenderState.clearColor.g, mRenderState.clearColor.b, mRenderState.clearColor.a);
            quad->draw();
        }

        quad->endDraw();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void BloomPass::endPass()
{
}

void BloomPass::resize(uint width, uint height)
{
    //resize viewport
    mRenderState.viewport.z = width;
    mRenderState.viewport.w = height;
    //resize fbo if nessesary
    if (!mDownSampleFBOs.empty())
    {
        for (auto& downSampleFBO : mDownSampleFBOs)
        {
            downSampleFBO->resize(width, height);
        }
    }

    if (!mUpSampleFBOs.empty())
    {
        for (auto& upSampleFBO : mUpSampleFBOs)
        {
            upSampleFBO->resize(width, height);
        }
    }
    mExtractHighLightFBO->resize(width, height);
}

Texture* BloomPass::getHDRTexture() const
{
    return mUpSampleFBOs[downSamples - 2]->getColorAttachment(0);
}

Texture* BloomPass::getLDRTexture() const
{
    return mExtractHighLightFBO->getColorAttachment(1);
}

FrameBuffer* BloomPass::getDebugView() const
{
    return mUpSampleFBOs[downSamples - 2].get();
}
