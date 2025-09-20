#include <gtc/type_ptr.hpp>
#include "cascadeShadowMapPass.h"
#include "graphic/gpuBuffer/uniformBuffer.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "graphic/program/program.h"
#include "scene/camera/camera.h"
#include "scene/scene.h"
#include "scene/renderObject.h"
#include "scene/light/light.h"

//NEED DEPTH TEST!!!
CascadeShadowMapPass::CascadeShadowMapPass(Scene* scene, const RenderState& state)
	:RenderPass(state),
	 mScene(scene)
{
	//lightMatrices uniform buffer
	mLightMatricesBuffer = std::make_unique<UniformBuffer>(16 * sizeof(glm::mat4), 2);
	auto camera = mScene->getCurrentCamera();
	float farPlane = camera->getFar();
	mCascadeFrustumDistances.push_back(farPlane / 50.0f);
	mCascadeFrustumDistances.push_back(farPlane / 40.0f);
	mCascadeFrustumDistances.push_back(farPlane / 10.0f);
	mCascadeFrustumDistances.push_back(farPlane / 5.0f);
	mCascadedLayerNum = mCascadeFrustumDistances.size();
	//texture array depthBuffer fbo
	std::initializer_list<FrameBufferSpecification> spec =
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
	mFrameBuffer = std::make_shared<FrameBuffer>(glm::vec3{state.width, state.height, mCascadedLayerNum + 1 }, spec);
	//update lightMatricesBuffer
	updateLightMatricesBuffer();
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
	Light* light = mScene->getLights()[mScene->getShadowLightIndex()].get();
	for(int i = 0; i < CascadeLayers; i++)
	{
		if (i == 0)
		{
			near = cameraNear;
			far = mCascadeFrustumDistances[i];
		}
		else if (i == CascadeLayers - 1)
		{
			near = mCascadeFrustumDistances[i - 1];
			far = cameraFar;
		}
		else
		{
			near = mCascadeFrustumDistances[i - 1];
			far = mCascadeFrustumDistances[i];
			
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
		glm::mat4 lightViewSpace = glm::lookAt(lightPos, lightDirection, { 0, 1, 0 });
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
		constexpr float zMult = 20.0f;
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

const std::vector<float>& CascadeShadowMapPass::getCascadedFrustumDistanes() const
{
	return mCascadeFrustumDistances;
}

int CascadeShadowMapPass::getCascadedLayerCount() const
{
	return mCascadedLayerNum;
}

int CascadeShadowMapPass::getShadowLightIndex() const
{
	return mScene->getShadowLightIndex();
}
