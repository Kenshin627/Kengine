#include "renderObject.h"
#include "camera/camera.h"
#include "light/pointLight/pointLight.h"
#include "light/spotLight/spotLight.h"
#include "scene.h"
#include "core.h"

Scene::Scene()
	  :mLightCount(0)
{
	/////////////////////////////////INIT UNIFORM BUFFER ////////////////////////////////////
	
	/////////////////////////////////////////////////////////////////////////////////////////
	//CAMERABUFFER BINDINGPOINT 0
	// /////////////////////////////////////////////////////////////////////////////////////
	// | VIEWPROJECTIONMATRIX mat4                                                  |
	// | PROJECTIONMATRIX     mat4                                                  |
	// | VIEWMATRIX			  mat4                                                  |
	// | POSITION vec3		       + padding float                                  |
	//viewprojectionMatrix + projectionMatrix + viewMatrix + position + padding
	mCameraBuffer = std::make_unique<UniformBuffer>(sizeof(glm::mat4) * 3 + sizeof(glm::vec4), 0); 

	/////////////////////////////////////////////////////////////////////////////////////////
	//LIGHTBUFFER BINDINGPOINT 1
	// /////////////////////////////////////////////////////////////////////////////////////
	// | POSITION  vec3 + padding	  float											|
	// | DIRECTION vec3 + padding	  float											|
	// | COLOR     vec3 + padding	  float											|
	// | ATTENTION vec3 + padding	  float											|
	// | TYPE float		+ INNERCUTOFF float + OUTTERCUTOFF float + padding          |
	// | LIGHTCOUNT float + padding + 3										        |
	mLightBuffer = std::make_unique<UniformBuffer>(MAX_LIGHTS * sizeof(GPULightBufferData), 1);
	mScreenQuad = std::make_unique<ScreenQuad>();
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

void Scene::addRenderObject(const std::vector<std::shared_ptr<RenderObject>>& objects)
{
	if (objects.size() == 0)
	{
		return;
	}
	mRenderList.insert(mRenderList.end(), objects.begin(), objects.end());
}

void Scene::addLight(std::shared_ptr<Light> light)
{
	addLight({ light });	
}

void Scene::addLights(const std::initializer_list<std::shared_ptr<Light>>& lights)
{
	if (mLights.size() + lights.size() > MAX_LIGHTS)
	{
		KS_CORE_WARN("[SCENE WARNING]: Exceed max point lights in scene!");
		return;
	}

	mLights.insert(mLights.begin(), lights.begin(), lights.end());
	mLightCount = mLights.size();
	//update light buffer
	std::vector<GPULightBufferData> gpuLightBufferData;
	gpuLightBufferData.reserve(mLightCount);

	for (auto& light : mLights)
	{
		GPULightBufferData lightData;
		lightData.position = glm::vec4(light->getPosition(), 0.0);
		lightData.direction = glm::vec4(glm::normalize(light->getDirection()), 0.0);
		lightData.color = glm::vec4(light->getColor(), 0.0);
		lightData.attentionFactor.x = light->getConstant();
		lightData.attentionFactor.y = light->getLinear();
		lightData.attentionFactor.z = light->getQuadratic();
		LightType type = light->getType();
		
		switch (type)
		{
		case LightType::PointLight:
		{
			lightData.type = LightType::PointLight;
			break;
		}
		case LightType::SpotLight:
		{
			auto spotLight = std::static_pointer_cast<SpotLight>(light);
			lightData.type = LightType::SpotLight;
			lightData.innerCutoff = spotLight->getInner();
			lightData.outterCutoff = spotLight->getOutter();
			break;
		}

		default:
		{
			KS_CORE_ERROR("unknown light type");
			break;
		}
		}
		lightData.lightCount = mLightCount;
		gpuLightBufferData.push_back(lightData);
	}

	mLightBuffer->setData(mLightCount * sizeof(GPULightBufferData), gpuLightBufferData.data());
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
		const glm::mat4& viewProj = mMainCamera->getViewProjectionMatrix();
		const glm::mat4& proj = mMainCamera->getProjectionMatrix();
		const glm::mat4& view = mMainCamera->getViewMatrix();
		glm::vec4 camPos = glm::vec4(mMainCamera->getPosition(), 1.0f);
		mCameraBuffer->setData(sizeof(glm::mat4), &viewProj, 0);
		mCameraBuffer->setData(sizeof(glm::mat4), &proj, sizeof(glm::mat4));
		mCameraBuffer->setData(sizeof(glm::mat4), &view, sizeof(glm::mat4) * 2);
		mCameraBuffer->setData(sizeof(glm::vec4), &camPos, sizeof(glm::mat4) * 3);
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

const std::vector<std::shared_ptr<Light>>& Scene::getLights() const
{
	return mLights;
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
		const glm::mat4& proj = mMainCamera->getProjectionMatrix();
		const glm::mat4& view = mMainCamera->getViewMatrix();
		glm::vec4 camPos = glm::vec4(mMainCamera->getPosition(), 1.0f);
		mCameraBuffer->setData(sizeof(glm::mat4), &viewProj, 0);
		mCameraBuffer->setData(sizeof(glm::mat4), &proj, sizeof(glm::mat4));
		mCameraBuffer->setData(sizeof(glm::mat4), &view, sizeof(glm::mat4) * 2);
		mCameraBuffer->setData(sizeof(glm::vec4), &camPos, sizeof(glm::mat4) * 3);
	}
	//update lightCount

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
		obj->beginDraw();
		obj->draw();
		obj->endDraw();
	}
}

void Scene::checkSceneReady() const
{
	if (!mMainCamera)
	{
		KS_CORE_ERROR("[SCENE ERROR]: No main camera in scene!");
		return;
	}
	if (mLights.size() == 0)
	{
		KS_CORE_WARN("[SCENE WARNING]: No point light in scene!");
		return;
	}
}

uint Scene::getLightCount() const
{
	return mLightCount;
}

ScreenQuad* Scene::getScreenQuad() const
{
	return mScreenQuad.get();
}

