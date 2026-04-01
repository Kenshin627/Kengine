#include "ssr.h"
#include "graphic/program/program.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "graphic/renderer/renderer.h"
#include "graphic/renderPass/deferredRendering/geometryPass.h"
#include "graphic/renderPass/deferredRendering/lightingPass.h"
#include "geometry/screenQuad.h"
#include "logger.h"
#include "scene/scene.h"

SSR::SSR(Renderer* r, const RenderState& state)
    :RenderPass(r, state)
{
    mProgram = std::make_shared<Program>();
    std::initializer_list<ShaderFile> shaders =
    {
        { "core/graphic/shaderSrc/ssr/vs.glsl", ShaderType::Vertex },
        { "core/graphic/shaderSrc/ssr/fs.glsl", ShaderType::Fragment }
    };
    mProgram->buildFromFiles(shaders);

    std::initializer_list<FrameBufferSpecification> spec =
    {
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
    mFrameBuffer = std::make_shared<FrameBuffer>(glm::vec3{ mSize.x, mSize.y ,0 }, spec);
}

SSR::~SSR()
{
}

void SSR::beginPass()
{
    RenderPass::beginPass();
    //uniform sampler2D  textureColor;
    //uniform sampler2D  texturePosInViewSpace;
    //uniform sampler2D  textureNormalInViewSpace;
    //uniform sampler2D  textureIsMirror;
    //uniform sampler2D  textureDepth;

    //layout(location = 0) out vec3  gPosition;    //position in viewSpace
    //layout(location = 1) out vec3  gNormal;        //normal in viewSpace
    //layout(location = 2) out vec4  gDiffuse;       //diffuse or emissive + isEmissive or albedo
    //layout(location = 3) out vec4  gSpecShiness; //spec + shiens  or metallic + roughness
    //layout(location = 4) out float gMaterialType; //material type blinnphong 0  pbr 1
    GeometryPass* gpass = static_cast<GeometryPass*>(mOwner->getRenderPass(RenderPassKey::GEOMETRY));
    Texture* pos = gpass->getCurrentFrameBuffer()->getColorAttachment(0);
    Texture* normal = gpass->getCurrentFrameBuffer()->getColorAttachment(1);
    Texture* mirror = gpass->getCurrentFrameBuffer()->getColorAttachment(5);
    Texture* depth = gpass->getCurrentFrameBuffer()->getDepthStencilAttachment();

    LightingPass* lightingPass = static_cast<LightingPass*>(mOwner->getRenderPass(RenderPassKey::DEFFEREDSHADING));
    Texture* color = lightingPass->getCurrentFrameBuffer()->getColorAttachment(0);

    color->bind(0);
    mProgram->setUniform("textureColor", 0);
    pos->bind(1);
    mProgram->setUniform("texturePosInViewSpace", 1);
    normal->bind(2);
    mProgram->setUniform("textureNormalInViewSpace", 2);
    mirror->bind(3);
    mProgram->setUniform("textureIsMirror", 3);
    depth->bind(4);
    mProgram->setUniform("textureDepth", 4);
}

void SSR::runPass(Scene* scene)
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

void SSR::endPass()
{
    RenderPass::endPass();
}