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
#include "postProcess/grayScaleEffect/grayScaleEffect.h"
#include "graphic/renderer/renderer.h"
#include "cannyEdgeDetection/sobel/sobelPass.h"
#include "cannyEdgeDetection/nms/nonMaxSuppression.h"
#include "cannyEdgeDetection/doubleThreshold/doubleThreshold.h"

class PassFactory
{
public:
	static std::unique_ptr<RenderPass> ceate(Renderer* r,RenderPassKey key, const RenderState& state)
	{
		switch (key)
		{
		case RenderPassKey::SSAO:
			return std::make_unique<SSAOPass>(SSAOSpecification{}, r, state);
		case RenderPassKey::SSAOBLUR:
			return std::make_unique<GaussianBlur>(GaussianBlurSpecification{}, r, state);
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
			return std::make_unique<GaussianBlur>(GaussianBlurSpecification{}, r, state);
		case RenderPassKey::GEOMETRY:
			return std::make_unique<GeometryPass>(r, state);
		case RenderPassKey::DEFFEREDSHADING:
			return std::make_unique<LightingPass>(r, state);
		case RenderPassKey::FORWARDSHADING:
			return std::make_unique<ForwardShadingPass>(r, state);
		case RenderPassKey::TONEMAPPING:
			return std::make_unique<ToneMapping>(1.0, r, state);
		case RenderPassKey::GRAYSCALER:
			return std::make_unique<GrayScaleEffect>(r, state);
		case RenderPassKey::SOBEL:
			return std::make_unique<SobelPass>(r, state);
		case RenderPassKey::NMS:
			return std::make_unique<NonMaxSuppression>(r, state);
		case RenderPassKey::DOUBLETHRESHOLD:
			return std::make_unique<DoubleThreshold>(r, state);
		default:
			break;
		}
	}
private:
	PassFactory();
};