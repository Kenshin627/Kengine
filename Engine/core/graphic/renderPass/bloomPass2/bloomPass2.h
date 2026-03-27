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

    Texture* getOutputTexture() const;
    FrameBuffer* getDebugView() const;
private:
    std::unique_ptr<Program> mDownSampleProgram;
    std::unique_ptr<Program> mUpSampleProgram;
    std::vector<std::unique_ptr<FrameBuffer>> mDownSampleFBOs;
    std::vector<std::unique_ptr<FrameBuffer>> mUpSampleFBOs;
};