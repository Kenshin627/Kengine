#include "graphic/renderPass/bloomPass/bloomPass.h"
#include "graphic/texture/texture2D/texture2D.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "graphic/renderer/renderer.h"
#include "graphic/program/program.h"
#include "geometry/screenQuad.h"
#include "gtao.h"
#include "scene/scene.h"
#include "core.h"
#include <graphic/renderPass/gaussianBlur/gaussianBlur.h>
#include "graphic/renderPass/BloomPass/BloomPass.h"
#include "graphic/renderPass/deferredRendering/geometryPass.h"
#include "scene/camera/camera.h"

#define HILBERT_LEVEL 6U
#define HILBERT_WIDTH (1U << HILBERT_LEVEL)
#define HILBERT_AREA (HILBERT_WIDTH * HILBERT_WIDTH)

GTAO::GTAO(Renderer* r, const RenderState& state)
    : RenderPass(r, state)
{
    generateHilbertLUT();
	mPreFilterDepthProgram = std::make_unique<Program>();
	std::initializer_list<ShaderFile> preFilterDepthShaders =
	{
		{ "core/graphic/shaderSrc/gtao/calcViewspaceZ.glsl", ShaderType::Compute }
		
	};
	mPreFilterDepthProgram->buildFromFiles(preFilterDepthShaders);

	mDepthMipChainProgram = std::make_unique<Program>();
	mDepthMipChainProgram->buildFromFiles({
		{ "core/graphic/shaderSrc/gtao/downsampleDepthMipchain.glsl", ShaderType::Compute }
	});

    mGTAOProgram = std::make_unique<Program>();
    mGTAOProgram->buildFromFiles({
        { "core/graphic/shaderSrc/gtao/gtao.glsl", ShaderType::Compute }
    });

    createTextures();
    mSettingBuffer = std::make_unique<UniformBuffer>(sizeof(GTAOSettings), 2);
}

GTAO::~GTAO()
{
}

void GTAO::beginPass()
{
    //
    mSettingBuffer->bind();
	mSettings.viewportPixelSize = glm::vec2(1.0f) / glm::vec2(mSize);
    //TODO
    //mSettings.depthUnpackConsts
    float fov = mOwner->getCurrentScene()->getCurrentCamera()->getFov();
	float aspectRatio = mOwner->getCurrentScene()->getCurrentCamera()->getAspectRatio();
	float tanHalfFovY = tan(glm::radians(fov * 0.5f));
    float tanHalfFovX = tanHalfFovY * aspectRatio;

	mSettings.ndcToViewMul = glm::vec2(2.0 * tanHalfFovX, 2.0 * tanHalfFovY);
	mSettings.ndcToViewAdd = glm::vec2(-tanHalfFovX, -tanHalfFovY);
	mSettings.ndcToViewMulXPixelSize = mSettings.ndcToViewMul * mSettings.viewportPixelSize;
    
    //mSettings.sliceCount = 9;
    //mSettings.stepsPerSlice = 9;
    
	mSettingBuffer->setData(sizeof(GTAOSettings), &mSettings);
}

void GTAO::endPass()
{
    //
    mSettingBuffer->unBind();
}

void GTAO::runPass(Scene* scene)
{
    //convert ndc depth to view depth.
    mPreFilterDepthProgram->bind();
	GeometryPass* gpass = static_cast<GeometryPass*>(mOwner->getRenderPass(RenderPassKey::GEOMETRY));
	Texture* depth = gpass->getCurrentFrameBuffer()->getDepthStencilAttachment();
	depth->bind(0);
	glm::vec2 pixelSize = glm::vec2(1.0f) / glm::vec2(mSize);	
    mPreFilterDepthProgram->setUniform("pixelSize", pixelSize);
	mPreFilterDepthProgram->setUniform("sourceNDCDepth", 0);
    mDepthMipChain->bindImage2D(0, 0, GL_WRITE_ONLY);
	glDispatchCompute((mSize.x + 7) / 8, (mSize.y + 7) / 8, 1); 
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    //dpeth mipmap chain generator.
	mDepthMipChainProgram->bind();
    mDepthMipChainProgram->setUniform("effectRadius", 0.5f);
    mDepthMipChainProgram->setUniform("radiusMultiplier", 1.457f);
    mDepthMipChainProgram->setUniform("effectFalloffRange", 0.615f);
	for (uint level = 1; level < mDepthMipChainLevels; ++level)
	{
		mDepthMipChain->bindImage2D(0, level, GL_WRITE_ONLY);
		mDepthMipChain->bindImage2D(1, level - 1, GL_READ_ONLY);
        uint width = depth->width() / (1 << level);
        uint height = depth->height() / (1 << level);
        glDispatchCompute((width +7) / 8, (height + 7) / 8, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
	//GTAO
    //layout(binding = 0, rgba32f)   uniform image2D viewSpaceNormalTex;
    //layout(binding = 1, r16)       uniform image2D hilbertLUT; //64 * 64
    //layout(binding = 2, rgba32f)   uniform image2D viewSpacePositionTex;
    //layout(binding = 3, r16f)      uniform image2D outEdgesTex;
    //layout(binding = 4, r32f)      uniform image2D outAOTex;
	mGTAOProgram->bind();
    Texture* viewPosition = gpass->getCurrentFrameBuffer()->getColorAttachment(0);
	Texture* viewNormal = gpass->getCurrentFrameBuffer()->getColorAttachment(1);
	viewNormal->bindImage2D(0, 0, GL_READ_ONLY);
	mHilbertLUT->bindImage2D(1, 0, GL_READ_ONLY);
	viewPosition->bindImage2D(2, 0, GL_READ_ONLY);
	mEdgesTex->bindImage2D(3, 0, GL_WRITE_ONLY);
	mGTAOTex->bindImage2D(4, 0, GL_WRITE_ONLY);
    mDepthMipChain->bind(0);
	mGTAOProgram->setUniform("depthTex", 0);
    glDispatchCompute((mSize.x + 7) / 8, (mSize.y + 7) / 8, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

Texture* GTAO::getDebugView() const
{
    return mFrameBuffer->getColorAttachment(0);
}

void GTAO::resize(uint width, uint height)
{
	mSize = { width, height };
    createTextures();
}

uint GTAO::hilbertIndex(uint posX, uint posY)
{
    uint index = 0U;
    for (uint curLevel = HILBERT_WIDTH / 2U; curLevel > 0U; curLevel /= 2U)
    {
        uint regionX = (posX & curLevel) > 0U;
        uint regionY = (posY & curLevel) > 0U;
        index += curLevel * curLevel * ((3U * regionX) ^ regionY);
        if (regionY == 0U)
        {
            if (regionX == 1U)
            {
                posX = uint((HILBERT_WIDTH - 1U)) - posX;
                posY = uint((HILBERT_WIDTH - 1U)) - posY;
            }

            uint temp = posX;
            posX = posY;
            posY = temp;
        }
    }
    return index;
}

void GTAO::generateHilbertLUT()
{
    constexpr int width = 64;
    uint16_t data[width * width];
    for (int x = 0; x < width; x++)
        for (int y = 0; y < width; y++)
        {
            uint r2index = hilbertIndex(x, y);
            KS_CORE_ASSERT(r2index < 65536, "hilbertIndex out of range.");
            data[x + width * y] = static_cast<uint16_t>(r2index);
        }

    TextureSpecification textureSpec;
    textureSpec.width = width;
    textureSpec.height = width;
    textureSpec.warpS = TextureWarpMode::REPEAT;
    textureSpec.warpT = TextureWarpMode::REPEAT;
    textureSpec.minFilter = TextureFilter::LINEAR;
    textureSpec.magFilter = TextureFilter::LINEAR;
    textureSpec.mipmapLevel = 1;
    textureSpec.internalFormat = TextureInternalFormat::R16UI;
    textureSpec.dataFormat = TextureDataFormat::R_INTERGER;
    textureSpec.chanel = 1;

    mHilbertLUT = std::make_unique<Texture2D>(textureSpec);
    mHilbertLUT->loadFromData(width, width, data, 1, TextureInternalFormat::R16UI, TextureDataFormat::R_INTERGER, GL_UNSIGNED_SHORT);
}

void GTAO::createTextures()
{
    TextureSpecification depthMipmapSpec;
    depthMipmapSpec.chanel = 1;
    depthMipmapSpec.dataFormat = TextureDataFormat::R;
    depthMipmapSpec.internalFormat = TextureInternalFormat::R32F;
    depthMipmapSpec.magFilter = TextureFilter::LINEAR;
    depthMipmapSpec.minFilter = TextureFilter::LINEAR_MIPMAP_LINEAR;
    depthMipmapSpec.depth = 1;
    depthMipmapSpec.warpS = TextureWarpMode::CLAMP_TO_EDGE;
    depthMipmapSpec.warpT = TextureWarpMode::CLAMP_TO_EDGE;
    depthMipmapSpec.width = mSize.x;
    depthMipmapSpec.height = mSize.y;
    depthMipmapSpec.mipmapLevel = mDepthMipChainLevels;

    mDepthMipChain = std::make_unique<Texture2D>(depthMipmapSpec);
    glTextureStorage2D(mDepthMipChain->id(), mDepthMipChainLevels, Texture::convertToGLInternalFormat(TextureInternalFormat::R32F), mSize.x, mSize.y);

	TextureSpecification gtaoSpec;
    gtaoSpec.chanel = 1;
    gtaoSpec.dataFormat = TextureDataFormat::R;
    gtaoSpec.internalFormat = TextureInternalFormat::R32F;
    gtaoSpec.magFilter = TextureFilter::LINEAR;
    gtaoSpec.minFilter = TextureFilter::LINEAR_MIPMAP_LINEAR;
    gtaoSpec.depth = 1;
    gtaoSpec.warpS = TextureWarpMode::CLAMP_TO_EDGE;
    gtaoSpec.warpT = TextureWarpMode::CLAMP_TO_EDGE;
    gtaoSpec.width = mSize.x;
    gtaoSpec.height = mSize.y;
    gtaoSpec.mipmapLevel = 1;
	mGTAOTex = std::make_unique<Texture2D>(gtaoSpec);
    glTextureStorage2D(mGTAOTex->id(), 1, Texture::convertToGLInternalFormat(TextureInternalFormat::R32F), mSize.x, mSize.y);
    mEdgesTex = std::make_unique<Texture2D>(gtaoSpec);
    glTextureStorage2D(mEdgesTex->id(), 1, Texture::convertToGLInternalFormat(TextureInternalFormat::R32F), mSize.x, mSize.y);
}
