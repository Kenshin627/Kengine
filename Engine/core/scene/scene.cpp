#include "renderObject.h"
#include "camera/camera.h"
#include "light/pointLight/pointLight.h"
#include "light/spotLight/spotLight.h"
#include "scene.h"
#include "core.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "scene/light/spotLight/spotLight.h"

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
	// | NEAR FAR	float * 2	   + padding float * 2                              |
	//viewprojectionMatrix + projectionMatrix + viewMatrix + position + padding
	mCameraBuffer = std::make_unique<UniformBuffer>(sizeof(glm::mat4) * 3 + sizeof(glm::vec4) * 2, 0); 

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
	object->setOwner(this);
}

void Scene::addRenderObject(const std::initializer_list<std::shared_ptr<RenderObject>>& objects)
{
	if (objects.size() == 0)
	{
		return;
	}
	mRenderList.insert(mRenderList.end(), objects.begin(), objects.end());
	for (auto& renderOject : objects)
	{
		renderOject->setOwner(this);
	}
}

void Scene::addRenderObject(const std::vector<std::shared_ptr<RenderObject>>& objects)
{
	if (objects.size() == 0)
	{
		return;
	}
	mRenderList.insert(mRenderList.end(), objects.begin(), objects.end());
	for (auto& renderOject : objects)
	{
		renderOject->setOwner(this);
	}
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

	for (auto& light : lights)
	{
		addRenderObject(light);
	}
	updateLightBuffer();
}

std::vector<std::shared_ptr<Light>> Scene::getLights()
{
	return mLights;
}

void Scene::setMainCamera(std::shared_ptr<Camera> camera)
{
	if (mMainCamera == camera)
	{
		return;
	}
	
	mMainCamera = camera;
	if (mMainCamera)
	{
		updateCameraBuffer();
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
		updateCameraBuffer();
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

void Scene::updateLightBuffer()
{
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
			lightData.innerCutoff = spotLight->getCosInner();
			lightData.outterCutoff = spotLight->getCosOutter();
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

void Scene::updateSceneUI()
{
	ImGui::Begin("Light Control");
	auto lights = mLights;
	for (int i = 0; i < lights.size(); i++)
	{
		ImGui::PushID(i);
		auto light = lights[i];
		std::string lightName = light->getType() == LightType::PointLight ? "PointLight" : "SpotLight";
		ImGui::Text(light->getName().c_str());
		glm::vec3 pos = light->getPosition();
		if (ImGui::DragFloat3("position", &(pos.r), 0.1f))
		{
			light->setPosition(pos);
		}
		glm::vec3 col = light->getColor();
		if (ImGui::ColorEdit3("color", &(col.r)))
		{
			light->setColor(col);
		}
		float constant = light->getConstant();
		if (ImGui::DragFloat("attention constant", &constant, 0.01, 0.0, 1.0))
		{
			light->setConstant(constant);
		}

		float linear = light->getLinear();
		if (ImGui::DragFloat("attention linear", &linear, 0.01, 0.0, 1.0))
		{
			light->setLinear(linear);
		}

		float quadratic = light->getQuadratic();
		if (ImGui::DragFloat("attention quadratic", &quadratic, 0.01, 0.0, 1.0))
		{
			light->setQuadratic(quadratic);
		}

		if (light->getType() == LightType::SpotLight)
		{
			glm::vec3 dir = light->getDirection();
			if (ImGui::DragFloat3("direction", &(dir.r), 0.1f))
			{
				light->setDirection(dir);
			}

			std::shared_ptr<SpotLight> spot = std::static_pointer_cast<SpotLight>(light);			
			float outter = spot->getOutter();
			if (ImGui::DragFloat("spot Outter cutoff", &outter, 0.1, 0.0))
			{
				spot->setOutter(outter);
			}
			float inner = spot->getInner();
			if (ImGui::DragFloat("spot Inner cutoff", &inner, 0.1, 0.0))
			{
				spot->setInner(inner);
			}
		}


		ImGui::PopID();
	}

	bool show = true;
	ImGui::ShowDemoWindow(&show);
	ImGui::End();

	ImGui::Begin("Camera Control");
	ImGui::Text("mainCamera");
	glm::vec3 pos = mMainCamera->getPosition();
	if (ImGui::DragFloat3("position", &pos.x, 0.1))
	{
		mMainCamera->setPosition(pos);
	}

	glm::vec3 center = mMainCamera->getCenter();
	if (ImGui::DragFloat3("focalPoint", &center.x))
	{
		mMainCamera->setFocalPoint(center);
	}

	glm::vec3 up = mMainCamera->getViewUp();
	if (ImGui::DragFloat3("viewUp", &up.x))
	{
		mMainCamera->setViewUp(up);
	}

	float fovAngle = mMainCamera->getFov();
	if (ImGui::DragFloat("fov", &fovAngle, 1.0, 0.0, 360.0))
	{
		mMainCamera->setFovAngle(fovAngle);
	}
	ImGui::End();

	ImGui::Begin("Scene Graph");

	for(int i = 0;i < mRenderList.size(); i++)
	{
		auto ro = mRenderList[i];
		ImGui::PushID(i);
		ImGui::Text(ro->getName().c_str());
		auto pos = ro->getPosition();
		if (ImGui::DragFloat3("position", &pos.x, 0.2))
		{
			ro->setPosition(pos);
		}
		ImGui::PopID();
	}
	ImGui::End();
}

//TODO: setting in ui
uint Scene::getShadowLightIndex() const
{	
	for (size_t i = 0; i < mLights.size(); i++)
	{
		auto l = mLights[i];
		if (l->isCastShadow())
		{
			return i;
		}
	}
	return -1;
}

void Scene::updateCameraBuffer()
{
	const glm::mat4& viewProj = mMainCamera->getViewProjectionMatrix();
	const glm::mat4& proj = mMainCamera->getProjectionMatrix();
	const glm::mat4& view = mMainCamera->getViewMatrix();
	glm::vec4 camPos = glm::vec4(mMainCamera->getPosition(), 1.0f);
	float nearPlane = mMainCamera->getNear();
	float farPlane = mMainCamera->getFar();
	glm::vec4 clipRnage{ nearPlane, farPlane, 0.0f, 0.0f };
	mCameraBuffer->setData(sizeof(glm::mat4), &viewProj, 0);
	mCameraBuffer->setData(sizeof(glm::mat4), &proj, sizeof(glm::mat4));
	mCameraBuffer->setData(sizeof(glm::mat4), &view, sizeof(glm::mat4) * 2);
	mCameraBuffer->setData(sizeof(glm::vec4), &camPos, sizeof(glm::mat4) * 3);
	mCameraBuffer->setData(sizeof(glm::vec4), &clipRnage, sizeof(glm::mat4) * 3 + sizeof(glm::vec4));
}

