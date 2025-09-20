#pragma once
#include <vector>
#include <memory>
#include "typedef.h"
#include "graphic/gpuBuffer/uniformBuffer.h"
#include "geometry/screenQuad.h"

constexpr int MAX_LIGHTS = 16;

class RenderObject;
class Camera;
class Light;

class Scene
{
public:
	Scene();
	~Scene() = default;
	void addRenderObject(std::shared_ptr<RenderObject> object);
	void addRenderObject(const std::initializer_list<std::shared_ptr<RenderObject>>& objects);
	void addRenderObject(const std::vector<std::shared_ptr<RenderObject>>& objects);
	void addLight(std::shared_ptr<Light> light);
	void addLights(const std::initializer_list<std::shared_ptr<Light>>& lights);
	std::vector<std::shared_ptr<Light>> getLights();
	void setMainCamera(std::shared_ptr<Camera> camera);
	std::shared_ptr<Camera> getCurrentCamera() const;
	const std::vector<std::shared_ptr<RenderObject>>& getRenderList() const;
	const std::vector<std::shared_ptr<Light>>& getLights() const;
	void beginScene();
	void endScene();
	void draw();
	void checkSceneReady() const;
	uint getLightCount() const;
	ScreenQuad* getScreenQuad() const;
	void updateLightBuffer();
	void updateSceneUI();
	float getHeightMapScale() const { return mHeightMapScale; }
	Light* getShadowLight() const;
private:
	std::vector<std::shared_ptr<RenderObject>> mRenderList;
	std::vector<std::shared_ptr<Light>>		   mLights;
	std::shared_ptr<Camera>					   mMainCamera;
	std::unique_ptr<UniformBuffer>			   mCameraBuffer;
	std::unique_ptr<UniformBuffer>			   mLightBuffer;
	uint									   mLightCount;
	std::unique_ptr<ScreenQuad>				   mScreenQuad;
	float									   mHeightMapScale { 0.1 };
};