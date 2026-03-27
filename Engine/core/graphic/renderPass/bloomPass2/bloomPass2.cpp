#include "core.h"
#include "BloomPass2.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include <graphic/program/program.h>
#include "geometry/screenQuad.h"
#include "scene/scene.h"
#include "graphic/texture/texture.h"

static int downSamples = 7;
static int downSize = 2;

BloomPass2::BloomPass2(Renderer* r, const RenderState& state)
    :RenderPass(r, state)
{
    mDownSampleProgram = std::make_unique<Program>();
    mUpSampleProgram = std::make_unique<Program>();

    mDownSampleProgram->buildFromFiles({
        { "core/graphic/shaderSrc/bloom2/quad.glsl", ShaderType::Vertex },
        { "core/graphic/shaderSrc/bloom2/downSample.glsl", ShaderType::Fragment }
        });

    mUpSampleProgram->buildFromFiles({
        { "core/graphic/shaderSrc/bloom2/quad.glsl", ShaderType::Vertex },
        { "core/graphic/shaderSrc/bloom2/upSample.glsl", ShaderType::Fragment }
        });

    mDownSampleFBOs.reserve(7);
    mUpSampleFBOs.reserve(7);
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
}

BloomPass2::~BloomPass2()
{
}

void BloomPass2::beginPass()
{
}

void BloomPass2::runPass(Scene* scene)
{
    ScreenQuad* quad = scene->getScreenQuad();
    if (!quad)
    {
        KS_CORE_ERROR("pass screen quad is null");
        return;
    }
    quad->beginDraw();

    // downSampleChains
    auto source = mPrevPass->getCurrentFrameBuffer()->getColorAttachment(0);
    mDownSampleProgram->bind();
    mDownSampleFBOs[0]->bind();
    source->bind();
    mDownSampleProgram->setUniform("uSourceTex", 0);
    mDownSampleProgram->setUniform("uDownSampleBlurSize", 5);
    mDownSampleProgram->setUniform("uDownSampleBlurSigma", 1);
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
    mUpSampleProgram->setUniform("uUpSampleBlurSize", 5);
    mUpSampleProgram->setUniform("uUpSampleBlurSigma", 1);

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

void BloomPass2::endPass()
{
}

Texture* BloomPass2::getOutputTexture() const
{
    return mUpSampleFBOs[downSamples - 2]->getColorAttachment(0);
}

FrameBuffer* BloomPass2::getDebugView() const
{
    return mUpSampleFBOs[downSamples - 2].get();
}
