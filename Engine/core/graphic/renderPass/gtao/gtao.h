#pragma once
#include "../renderPass.h"

struct GTAOSettings
{
    float         effectRadius{0.5f};
    float         radiusMultiplier{ 1.457f };
    float         effectFalloffRange{ 0.615f };
    float         sampleDistributionPower{ 2.0f };
    float         thinOccluderCompensation{ 0.0f };
    int           sliceCount{9}; //2 3 9
    int           stepsPerSlice{3}; // 2 3
    float         depthMIPSamplingOffset{ 3.30f };
    int           depthMipMevls{5};
    float         finalValuePow{ 2.2f };
    float         occlusionTermScale{1.5f};
    float         padding0;

    glm::vec2     ndcToViewMulXPixelSize;
    glm::vec2     ndcToViewMul;
    glm::vec2     ndcToViewAdd;
    glm::vec2     viewportPixelSize;
    glm::vec2     depthUnpackConsts;
    glm::vec2     padding1;
};

class GTAO : public RenderPass
{
public:
    GTAO(Renderer* r, const RenderState& state);
    ~GTAO();
    virtual void beginPass() override;
    virtual void endPass() override;
    virtual void runPass(Scene* scene) override;
    virtual Texture* getDebugView() const override;
    virtual void resize(uint width, uint height) override;
private:
    uint hilbertIndex(uint posX, uint posY);
    void generateHilbertLUT();
	void createTextures();
private:
    std::unique_ptr<Texture2D>     mHilbertLUT;
	std::unique_ptr<Texture2D>     mDepthMipChain;
    std::unique_ptr<Program>       mPreFilterDepthProgram;
    std::unique_ptr<Program>       mDepthMipChainProgram;
    std::unique_ptr<Program>       mGTAOProgram;
    std::unique_ptr<Program>       mDeNoiseProgram;
    std::unique_ptr<Texture2D>     mGTAOTex;
    std::unique_ptr<Texture2D>     mEdgesTex;
    uint                           mDepthMipChainLevels{5};
    GTAOSettings                   mSettings;
    std::unique_ptr<UniformBuffer> mSettingBuffer;

};