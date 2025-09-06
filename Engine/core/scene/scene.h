#pragma once
#include <vector>
#include <memory>
#include "typedef.h"
#include "graphic/gpuBuffer/uniformBuffer.h"

constexpr int MAX_LIGHTS = 8;

class RenderObject;
class Camera;
class PointLight;

class Scene
{
public:
	Scene();
	~Scene() = default;
	void addRenderObject(std::shared_ptr<RenderObject> object);
	void addRenderObject(const std::initializer_list<std::shared_ptr<RenderObject>>& objects);
	void addPointLight(std::shared_ptr<PointLight> light);
	void setMainCamera(std::shared_ptr<Camera> camera);
	const std::vector<std::shared_ptr<RenderObject>>& getRenderList() const;
	const std::vector<std::shared_ptr<PointLight>>& getPointLights() const;
	void beginScene();
	void endScene();
	void draw();
	void checkSceneReady() const;
	uint getLightCount() const;
private:
	std::vector<std::shared_ptr<RenderObject>> mRenderList;
	std::vector<std::shared_ptr<PointLight>>   mPointLights;
	std::shared_ptr<Camera>					   mMainCamera;
	std::unique_ptr<UniformBuffer>			   mCameraBuffer;
	std::unique_ptr<UniformBuffer>			   mLightBuffer;
	uint									   mLightCount;
};