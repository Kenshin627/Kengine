#pragma once
#include <memory>
#include "renderPass.h"
#include "ssaoPass/ssaoPass.h"
#include "cascadeShadowMapPass/cascadeShadowMapPass.h"
#include "bloomPass/bloomPass.h"
#include "blurPass/blurPass.h"
#include "deferredRendering/geometryPass.h"
#include "deferredRendering/lightingPass.h"
#include "forwardShading/forwardShadingPass.h"
#include "toneMapping/toneMapping.h"
#include "gaussianBlur/gaussianBlur.h"
#include "graphic/renderer/renderer.h"

class PassFactory
{
public:
	static std::unique_ptr<RenderPass> ceate(Renderer* r,RenderPassKey key, const RenderState& state)
	{
		switch (key)
		{
		case RenderPassKey::SSAO:
			return std::make_unique<SSAOPass>(r, state, 128, 1.0);
		case RenderPassKey::SSAOBLUR:
			return std::make_unique<BlurPass>(4, r, state);
		case RenderPassKey::CSM:
			CascadeShadowMapPassSpecification spec;
			spec.cascadedLayer = 4;
			spec.pcfSize = 2;
			spec.splitLambda = 0.95;
			spec.splitMethod = FrustumSplitMethod::Pratical;
			return std::make_unique<CascadeShadowMapPass>(spec, r, state);
		case RenderPassKey::BLOOM:
			return std::make_unique<BloomPass>(r, state);
		case RenderPassKey::BLOOMBLUR:
			return std::make_unique<GaussianBlur>(6, r, state);
		case RenderPassKey::GEOMETRY:
			return std::make_unique<GeometryPass>(r, state);
		case RenderPassKey::DEFFEREDSHADING:
			return std::make_unique<LightingPass>(r, state);
		case RenderPassKey::FORWARDSHADING:
			return std::make_unique<ForwardShadingPass>(r, state);
		case RenderPassKey::TONEMAPPING:
			return std::make_unique<ToneMapping>(1.0, r, state);
		default:
			break;
		}
	}
private:
	PassFactory();
};