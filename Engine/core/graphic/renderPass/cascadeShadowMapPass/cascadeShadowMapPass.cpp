#include <gtc/type_ptr.hpp>
#include "cascadeShadowMapPass.h"
#include "graphic/gpuBuffer/uniformBuffer.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "graphic/program/program.h"
#include "scene/camera/camera.h"
#include "scene/scene.h"
#include "scene/renderObject.h"
#include "scene/light/light.h"
#include "imgui.h"

CascadeShadowMapPass::CascadeShadowMapPass(const CascadeShadowMapPassSpecification& spec, const RenderState& state)
	:RenderPass(state),
	 mScene(spec.scene),
	 mPcfSize(spec.pcfSize),
	 mCascadedLayer(spec.cascadedLayer),
	 mSplitLambda(spec.splitLambda),
	 mSplitMethod(spec.splitMethod)
{
	//lightMatrices uniform buffer bindingPoint  = 2
	mLightMatricesBuffer = std::make_unique<UniformBuffer>(16 * sizeof(glm::mat4), 2);
	cascadedSplit();
	//texture array depthBuffer fbo
	std::initializer_list<FrameBufferSpecification> fboSpec =
	{
		//TODO: becaz drawbuffers need a color attachment, furture will remove this color attachment
		{
			AttachmentType::Color,
			TextureInternalFormat::R8,
			TextureDataFormat::R,
			TextureWarpMode::CLAMP_TO_BORDER,
			TextureWarpMode::CLAMP_TO_BORDER,
			TextureFilter::NEAREST,
			TextureFilter::NEAREST,
			{1.0, 1.0, 1.0, 1.0}
		},
		{
			AttachmentType::Depth,
			TextureInternalFormat::DEPTH32,
			TextureDataFormat::DEPTH,
			TextureWarpMode::CLAMP_TO_BORDER,
			TextureWarpMode::CLAMP_TO_BORDER,
			TextureFilter::NEAREST,
			TextureFilter::NEAREST,
			{1.0, 1.0, 1.0, 1.0}
		}
	};
	mFrameBuffer = std::make_shared<FrameBuffer>(glm::vec3{state.width, state.height, mCascadedLayer }, fboSpec);
	//program
	mProgram = std::make_shared<Program>();
	std::initializer_list<ShaderFile> files =
	{
		{ "core/graphic/shaderSrc/cascadeShadowMap/vs.glsl", ShaderType::Vertex   },
		{ "core/graphic/shaderSrc/cascadeShadowMap/gs.glsl", ShaderType::Geometry },
		{ "core/graphic/shaderSrc/cascadeShadowMap/fs.glsl", ShaderType::Fragment }
	};
	mProgram->buildFromFiles(files);		
}

CascadeShadowMapPass::~CascadeShadowMapPass()
{

}

void CascadeShadowMapPass::beginPass()
{
	RenderPass::beginPass();
	//bind lightMatricesBuffer
	mLightMatricesBuffer->bind();
}

void CascadeShadowMapPass::runPass(Scene* scene)
{
	for (auto& renderObject : scene->getRenderList())
	{
		if (renderObject->getType() != RenderObjectType::Mesh)
		{
			continue;
		}
		renderObject->beginDraw(mProgram.get());
		renderObject->draw();
		renderObject->endDraw(mProgram.get());
	}
}

void CascadeShadowMapPass::setSplitLambda(float lambda)
{
	if (mSplitMethod != FrustumSplitMethod::Pratical)
	{
		return;
	}
	if (mSplitLambda != lambda)
	{
		mSplitLambda = lambda;
		cascadedSplit();
	}
}

void CascadeShadowMapPass::setPcfSize(int size)
{
	mPcfSize = size;
}

void CascadeShadowMapPass::cascadedSplit()
{
	switch (mSplitMethod)
	{
	case FrustumSplitMethod::Uniform:
		uniformSplit();
		break;
	case FrustumSplitMethod::Log:
		logarithmicSplit();
		break;
	case FrustumSplitMethod::Pratical:
		practicalSplit();
		break;
	default:
		break;
	}
	updateLightMatricesBuffer();
}

void CascadeShadowMapPass::uniformSplit()
{
	mCascadedFrustumSplit.clear();
	mCascadedFrustumSplit.reserve(mCascadedLayer);
	auto camera = mScene->getCurrentCamera();
	float near = camera->getNear();
	float far = camera->getFar();
	float range = far - near;
	float ratio = far / near;
	for (int i = 0; i < mCascadedLayer; i++)
	{
		float p = (i + 1) / static_cast<float>(mCascadedLayer);
		float uniform = near + range * p;
		mCascadedFrustumSplit.push_back(uniform);
	}	
}

void CascadeShadowMapPass::logarithmicSplit()
{
	mCascadedFrustumSplit.clear();
	mCascadedFrustumSplit.reserve(mCascadedLayer);
	auto camera = mScene->getCurrentCamera();
	float near = camera->getNear();
	float far = camera->getFar();
	float range = far - near;
	float ratio = far / near;
	for (int i = 0; i < mCascadedLayer; i++)
	{
		float p = (i + 1) / static_cast<float>(mCascadedLayer);
		float log = near * std::pow(ratio, p);
		mCascadedFrustumSplit.push_back(log);
	}
}

void CascadeShadowMapPass::practicalSplit()
{
	mCascadedFrustumSplit.clear();
	mCascadedFrustumSplit.reserve(mCascadedLayer);
	auto camera = mScene->getCurrentCamera();
	float near = camera->getNear();
	float far = camera->getFar();
	float range = far - near;
	float ratio = far / near;
	for (int i = 0; i < mCascadedLayer; i++)
	{
		float p		  = (i + 1) / static_cast<float>(mCascadedLayer);
		float uniform = near + range * p;
		float log	  = near * std::pow(ratio, p);
		float d		  = mSplitLambda * (log - uniform) + uniform;
		mCascadedFrustumSplit.push_back(d);
	}
}

std::vector<glm::vec4> CascadeShadowMapPass::getFrustumWorldSpaceCorners(float near, float far)
{
	auto camera = mScene->getCurrentCamera();
	const glm::mat4& viewMatrix = camera->getViewMatrix();
	float fov = camera->getFov();
	float aspectRatio = camera->getAspectRatio();
	glm::mat4 projectMatrix = glm::perspective(fov, aspectRatio, near, far);
	glm::mat4 inverseProjectionViewMatrix = glm::inverse(projectMatrix * viewMatrix);
	std::vector<glm::vec4> cornerInWorldSpace;
	for (int x = 0; x < 2; x++)
	{
		for (int y = 0; y < 2; y++)
		{
			for (int z = 0; z < 2; z++)
			{
				glm::vec4 corner = { x * 2.0 - 1.0, y * 2.0 - 1.0, z * 2.0 - 1.0, 1.0 };
				glm::vec4 worldSpaceCorner = inverseProjectionViewMatrix * corner;
				worldSpaceCorner /= worldSpaceCorner.w;
				cornerInWorldSpace.emplace_back(worldSpaceCorner);
			}
		}
	}
	return cornerInWorldSpace;
}

glm::mat4 CascadeShadowMapPass::getFrustaProjectionViewMatrix()
{
	return glm::mat4();
}

void CascadeShadowMapPass::updateLightMatricesBuffer()
{
	float near;
	float far;
	auto camera = mScene->getCurrentCamera();
	float cameraNear = camera->getNear();
	float cameraFar = camera->getFar();
	int lightIndex = mScene->getShadowLightIndex();
	if (lightIndex == -1)
	{
		//TODO
		return;
	}
	Light* light = mScene->getLights().at(lightIndex).get();
	for(int i = 0; i < mCascadedLayer; i++)
	{
		if (i == 0)
		{
			near = cameraNear;
			far = mCascadedFrustumSplit[i];
		}
		else
		{
			near = mCascadedFrustumSplit[i - 1];
			far = mCascadedFrustumSplit[i];
			
		}
		auto worldSpaceCorners = getFrustumWorldSpaceCorners(near, far);
		glm::vec3 center{0.0f};
		for (auto& corner : worldSpaceCorners)
		{
			center += glm::vec3(corner);
		}
		center /= worldSpaceCorners.size();
		glm::vec3 lightPos = light->getPosition();
		glm::vec3 lightDirection = light->getDirection();
		//TODO calc light Eye position
		glm::mat4 lightViewSpace = glm::lookAt(center - lightDirection, center, { 0, 1, 0 });
		//transform worldspace corners to lightViewSpace calc frustum
		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::lowest();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::lowest();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::lowest();
		for (const auto& corner : worldSpaceCorners)
		{
			const auto lightViewCorner = lightViewSpace * corner;
			minX = std::min(minX, lightViewCorner.x);
			maxX = std::max(maxX, lightViewCorner.x);
			minY = std::min(minY, lightViewCorner.y);
			maxY = std::max(maxY, lightViewCorner.y);
			minZ = std::min(minZ, lightViewCorner.z);
			maxZ = std::max(maxZ, lightViewCorner.z);
		}

		// Tune this parameter according to the scene
		constexpr float zMult = 4.0f;
		if (minZ < 0)
		{
			minZ *= zMult;
		}
		else
		{
			minZ /= zMult;
		}
		if (maxZ < 0)
		{
			maxZ /= zMult;
		}
		else
		{
			maxZ *= zMult;
		}

		const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
		mLightMatricesBuffer->setData(sizeof(glm::mat4), glm::value_ptr(lightProjection * lightViewSpace), i * sizeof(glm::mat4));
	}
}

void CascadeShadowMapPass::renderUI()
{
	//set Split Method
	ImGui::Begin("Cascaded Shadow Mapping Settings");

	int currentSplitMethodIndx = static_cast<int>(mSplitMethod);
	if (ImGui::Combo(splitMethods[currentSplitMethodIndx], &currentSplitMethodIndx, splitMethods, IM_ARRAYSIZE(splitMethods))) {
		selectSplitMethod(static_cast<FrustumSplitMethod>(currentSplitMethodIndx));
	}
	//display cascaded color
	bool showCsmColor = mDisplayCacadedColor;
	if (ImGui::Checkbox("display Cascaded Color", &showCsmColor))
	{
		setDisplayCacadedColor(showCsmColor);
	}
	//splitLambda
	float splitLambda = mSplitLambda;
	if (ImGui::DragFloat("splitLambda", &splitLambda, 0.001, 0.0, 1.0))
	{
		setSplitLambda(splitLambda);
	}

	//enable pcf or not
	bool enablePcf = mEnablePcf;
	if (ImGui::Checkbox("PCF", &enablePcf))
	{
		setEnablePCF(enablePcf);
	}

	//pcfSize
	if (enablePcf)
	{
		int pcfSize = mPcfSize;
		if (ImGui::DragInt("PCFSize", &pcfSize, 1, 1, 16))
		{
			setPcfSize(pcfSize);
		}
	}

	//show depthMap in another viewport
	ImGui::End();
}

void CascadeShadowMapPass::setDisplayCacadedColor(bool show)
{
	mDisplayCacadedColor = show;
}

void CascadeShadowMapPass::setEnablePCF(bool enable)
{
	mEnablePcf = enable;
}

const std::vector<float>& CascadeShadowMapPass::getCascadedFrustumDistanes() const
{
	return mCascadedFrustumSplit;
}

int CascadeShadowMapPass::getCascadedLayerCount() const
{
	return mCascadedLayer;
}

int CascadeShadowMapPass::getShadowLightIndex() const
{
	return mScene->getShadowLightIndex();
}

int CascadeShadowMapPass::getPcfSize() const
{
	return mPcfSize;
}

void CascadeShadowMapPass::selectSplitMethod(FrustumSplitMethod method)
{
	if (mSplitMethod != method)
	{
		mSplitMethod = method;
		cascadedSplit();
	}
}
