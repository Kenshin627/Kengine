#include "renderObject.h"
#include "camera/camera.h"
#include "light/pointLight/pointLight.h"
#include "scene.h"
#include "core.h"

Scene::Scene()
	  :mLightCount(0)
{
	/////////////////////////////////INIT UNIFORM BUFFER ///////////////////////////////////
	//camera buffer bindingpoint 0
	//viewprojectionMatrix  + position + padding
	mCameraBuffer = std::make_unique<UniformBuffer>(sizeof(glm::mat4) + sizeof(glm::vec4), 0); 
	//light buffer bindingpoint 1
	//position + padding  + color + padding + constant + linear + quadratic + padding + lightingCount + padding * 3
	mLightBuffer = std::make_unique<UniformBuffer>(MAX_LIGHTS * (sizeof(glm::vec4) * 3) + sizeof(int), 1);
}

void Scene::addRenderObject(std::shared_ptr<RenderObject> object)
{
	mRenderList.push_back(object);
}

void Scene::addRenderObject(const std::initializer_list<std::shared_ptr<RenderObject>>& objects)
{
	if (objects.size() == 0)
	{
		return;
	}
	mRenderList.insert(mRenderList.end(), objects.begin(), objects.end());
}

void Scene::addPointLight(std::shared_ptr<PointLight> light)
{
	if (mPointLights.size() == MAX_LIGHTS)
	{
		KS_CORE_WARN("[SCENE WARNING]: Exceed max point lights in scene!");
		return;
	}

	mPointLights.push_back(light);
	mLightCount = static_cast<uint>(mPointLights.size());
	//update light buffer
	std::vector<glm::vec4> lightData;
	lightData.resize(mLightCount * 3);
	for (size_t i = 0; i < mPointLights.size(); i++)
	{
		lightData[i * 3 + 0] = glm::vec4(mPointLights[i]->getPosition(), 1.0f);
		lightData[i * 3 + 1] = glm::vec4(mPointLights[i]->getColor(), 1.0f);
		lightData[i * 3 + 2] = glm::vec4(mPointLights[i]->getConstant(), mPointLights[i]->getLinear(), mPointLights[i]->getQuadratic(), 1.0f);
	}

	mLightBuffer->setData(sizeof(glm::vec4) * 3 * mLightCount, lightData.data(), 0);
	mLightBuffer->setData(sizeof(int), &mLightCount, sizeof(glm::vec4) * 3 * mLightCount);
}

void Scene::setMainCamera(std::shared_ptr<Camera> camera)
{
	if (mMainCamera == camera)
	{
		return;
	}
	
	mMainCamera = camera;
	//update camera buffer
	if (mMainCamera)
	{
		glm::mat4 viewProj = mMainCamera->getViewProjectionMatrix();
		glm::vec4 camPos = glm::vec4(mMainCamera->getPosition(), 1.0f);
		mCameraBuffer->setData(sizeof(glm::mat4), &viewProj, 0);
		mCameraBuffer->setData(sizeof(glm::vec4), &camPos, sizeof(glm::mat4));
	}
}

std::shared_ptr<Camera> Scene::getCurrentCamera() const
{
	return mMainCamera;
}

const std::vector<std::shared_ptr<RenderObject>>& Scene::getRenderList() const
{
	return mRenderList;
}

const std::vector<std::shared_ptr<PointLight>>& Scene::getPointLights() const
{
	return mPointLights;
}

void Scene::beginScene()
{
	checkSceneReady();
	mCameraBuffer->bind();
	mLightBuffer->bind();
	//update cameraBuffer if nessesary
	if (mMainCamera && mMainCamera->isCameraUniformDirty())
	{
		glm::mat4 viewProj = mMainCamera->getViewProjectionMatrix();
		glm::vec4 camPos = glm::vec4(mMainCamera->getPosition(), 1.0f);
		mCameraBuffer->setData(sizeof(glm::mat4), &viewProj, 0);
		mCameraBuffer->setData(sizeof(glm::vec4), &camPos, sizeof(glm::mat4));
	}
	//TODO: remove this to pass specification
	glEnable(GL_DEPTH_TEST);
}

void Scene::endScene()
{
	checkSceneReady();
	mCameraBuffer->unBind();
	mLightBuffer->unBind();
}

void Scene::draw()
{
	checkSceneReady();
	for (const auto& obj : mRenderList)
	{
		obj->draw();
	}
}

void Scene::checkSceneReady() const
{
	if (!mMainCamera)
	{
		KS_CORE_ERROR("[SCENE ERROR]: No main camera in scene!");
		return;
	}
	if (mPointLights.size() == 0)
	{
		KS_CORE_WARN("[SCENE WARNING]: No point light in scene!");
		return;
	}
}

uint Scene::getLightCount() const
{
	return mLightCount;
}

