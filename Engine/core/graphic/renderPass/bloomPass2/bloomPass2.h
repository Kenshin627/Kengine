#pragma once
#include "../renderPass.h"

class Texture;

class BloomPass2 : public RenderPass
{
public:
    BloomPass2(Renderer* r, const RenderState& state);
    ~BloomPass2();

    virtual void beginPass() override;
    virtual void runPass(Scene* scene) override;
    virtual void endPass() override;
    virtual void resize(uint width, uint height) override;
    Texture* getHDRTexture() const;
    Texture* getLDRTexture() const;
    FrameBuffer* getDebugView() const;

    void setThresholdMin(float t) { mThresholdMin = t; }
    float getThresholdMin() const { return mThresholdMin; }
    void setThresholdMax(float t) { mThresholdMax = t; }
    float getThresholdMax() const { return mThresholdMax; }

    void setBlurRadius(int r) { mBlurRadius = r; };
    int getBlurRadius() const { return mBlurRadius; };

    void setGaussianSigma(float sigma) { mGaussianSigma = sigma; };
    float getGaussianSigma() const { return mGaussianSigma; };

    void setBloomIntensity(float intensity) { mBloomIntensity = intensity; };
    float getBloomIntensity() const { return mBloomIntensity; };
private:
    std::unique_ptr<Program> mDownSampleProgram;
    std::unique_ptr<Program> mUpSampleProgram;
    std::vector<std::unique_ptr<FrameBuffer>> mDownSampleFBOs;
    std::vector<std::unique_ptr<FrameBuffer>> mUpSampleFBOs;

    std::unique_ptr<Program> mExtractHighLightProgram;
    std::unique_ptr<FrameBuffer> mExtractHighLightFBO;

    float mThresholdMin{ 0.8f };
    float mThresholdMax{ 1.2f };

    int mBlurRadius{ 5 };
    float mGaussianSigma{ 1.0f };
    float mBloomIntensity{ 1.0f };
};