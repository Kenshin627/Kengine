#pragma once
#include "../RenderPass.h"

class Texture;
class WeightedBlendedOIT : public RenderPass
{
public:
    WeightedBlendedOIT(Renderer* r, const RenderState& state);
    virtual ~WeightedBlendedOIT();
    virtual void beginPass() override;
    virtual void runPass(Scene* scene) override;
    virtual void endPass() override;
    virtual void resize(uint width, uint height) override;

private:
    std::unique_ptr<Program> mAccumulateProgram;
    std::unique_ptr<Program> mCompositeProgram;
    std::unique_ptr<FrameBuffer> mAccumulateFBO;
};