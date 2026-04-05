#include "core.h"
#include "weightedBlendedOIT.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include <graphic/program/program.h>
#include "geometry/screenQuad.h"
#include "scene/scene.h"
#include "graphic/texture/texture.h"
#include "graphic/renderPass/deferredRendering/geometryPass.h"
#include "graphic/renderer/renderer.h"
#include "scene/renderObject.h"
#include "graphic/renderPass/deferredRendering/lightingPass.h"

WeightedBlendedOIT::WeightedBlendedOIT(Renderer* r, const RenderState& state)
    :RenderPass(r, state)
{
    mAccumulateProgram = std::make_unique<Program>();
    mAccumulateProgram->buildFromFiles({
        { "core/graphic/shaderSrc/weightedBlendedOIT/accum/vs.glsl", ShaderType::Vertex },
        { "core/graphic/shaderSrc/weightedBlendedOIT/accum/fs.glsl", ShaderType::Fragment }
    });

    mCompositeProgram = std::make_unique<Program>();
    mCompositeProgram->buildFromFiles({
        { "core/graphic/shaderSrc/weightedBlendedOIT/composite/vs.glsl", ShaderType::Vertex },
        { "core/graphic/shaderSrc/weightedBlendedOIT/composite/fs.glsl", ShaderType::Fragment }
    });

    std::initializer_list<FrameBufferSpecification> accumSpecs =
    {
        //color attachment 0 - accumulate
        {
            AttachmentType::Color,
            TextureInternalFormat::RGBA16F,
            TextureDataFormat::RGBA,
            TextureWarpMode::CLAMP_TO_EDGE,
            TextureWarpMode::CLAMP_TO_EDGE,
            TextureFilter::LINEAR,
            TextureFilter::LINEAR
        },
        //color attachment 1 - revealage
        {
            AttachmentType::Color,
            TextureInternalFormat::R8,
            TextureDataFormat::R,
            TextureWarpMode::CLAMP_TO_EDGE,
            TextureWarpMode::CLAMP_TO_EDGE,
            TextureFilter::LINEAR,
            TextureFilter::LINEAR
        },
        {
            AttachmentType::Depth,
            TextureInternalFormat::DEPTH32,
            TextureDataFormat::DEPTH,
            TextureWarpMode::CLAMP_TO_EDGE,
            TextureWarpMode::CLAMP_TO_EDGE,
            TextureFilter::LINEAR,
            TextureFilter::LINEAR
        }
    };

    //TODO:Blit depth
    mAccumulateFBO = std::make_unique<FrameBuffer>(glm::vec3{ state.viewport.z,state.viewport.w, 0 }, accumSpecs);
	mRenderPassKey = RenderPassKey::WEIGHTEDBLENDEDOIT;
}

WeightedBlendedOIT::~WeightedBlendedOIT()
{
}

void WeightedBlendedOIT::beginPass()
{
}

void WeightedBlendedOIT::runPass(Scene* scene)
{
    auto width = mAccumulateFBO->getColorAttachment(0)->width();
    auto height = mAccumulateFBO->getColorAttachment(0)->height();

    //accumulate
    auto gPass = static_cast<GeometryPass*>(mOwner->getRenderPass(RenderPassKey::GEOMETRY));
    FrameBuffer* fbo = gPass->getCurrentFrameBuffer();
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo->id());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mAccumulateFBO->id());
    GLCALL(glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST));

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(false);
    glEnable(GL_BLEND);
    glBlendFunci(0, GL_ONE, GL_ONE);
    glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
    glBlendEquation(GL_ADD);
    mAccumulateProgram->bind();
    mAccumulateFBO->bind();
    glViewport(0, 0, width, height);
    float clearColor0[4] = { 0.0f };
    float clearColor1[4] = { 1.0f };
    glClearBufferfv(GL_COLOR, 0, clearColor0);
    glClearBufferfv(GL_COLOR, 1, clearColor1);

    auto& transparencyList = scene->getTransparencyList();
    for (auto& obj : transparencyList)
    {
        obj->beginDraw(mAccumulateProgram.get());
        obj->draw();
        obj->endDraw(mAccumulateProgram.get());
    }

    //composite
    ScreenQuad* quad = scene->getScreenQuad();
    if (!quad)
    {
        KS_CORE_ERROR("pass screen quad is null");
        return;
    }

    auto lightingPass = static_cast<LightingPass*>(mOwner->getRenderPass(RenderPassKey::DEFFEREDSHADING));
    FrameBuffer* opaqueFBO = lightingPass->getCurrentFrameBuffer();
    opaqueFBO->bind();
    mCompositeProgram->bind();

    mAccumulateFBO->getColorAttachment(0)->bind(0);
    mAccumulateProgram->setUniform("accum", 0);
    mAccumulateFBO->getColorAttachment(1)->bind(1);
    mAccumulateProgram->setUniform("reveal", 1);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0, 0, width, height);
    quad->beginDraw();
    quad->draw();
    quad->endDraw();
}

void WeightedBlendedOIT::endPass()
{
}

void WeightedBlendedOIT::resize(uint width, uint height)
{
    mAccumulateFBO->resize(width, height);
}