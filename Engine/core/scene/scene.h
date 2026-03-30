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
class Renderer;
class Animator;
class Animation;
class Model;

class Scene
{
public:
	Scene(Renderer* r);
	~Scene() = default;
	void addRenderObject(std::shared_ptr<RenderObject> object);
	void addRenderObject(const std::initializer_list<std::shared_ptr<RenderObject>>& objects);
	void addRenderObject(const std::vector<std::shared_ptr<RenderObject>>& objects);
	void addModel(const std::shared_ptr<Model> model);
	void addLight(std::shared_ptr<Light> light);
	void addLights(const std::initializer_list<std::shared_ptr<Light>>& lights);
	std::vector<std::shared_ptr<Light>> getLights();
	void setMainCamera(std::shared_ptr<Camera> camera);
	std::shared_ptr<Camera> getCurrentCamera() const;
	const std::vector<std::shared_ptr<RenderObject>>& getRenderList() const;
	const std::vector<std::shared_ptr<Light>>& getLights() const;
	void beginScene(double deltaTime);
	void endScene();
	void draw();
	void checkSceneReady() const;
	uint getLightCount() const;
	ScreenQuad* getScreenQuad() const;
	void updateLightBuffer();
	void updateSceneUI();
	int getShadowLightIndex() const;
	void playAnimation(Animation* animation);
	void addTransparencyObject(std::shared_ptr<RenderObject> obj);
	void addTransparencyObject(const std::initializer_list<std::shared_ptr<RenderObject>>& objects);
	void addTransparencyObject(const std::vector<std::shared_ptr<RenderObject>>& objects);
	const std::vector<std::shared_ptr<RenderObject>>& getTransparencyList() const;
	private:
		void updateCameraBuffer();
		void updateAnimationBuffer();
private:
	std::vector<std::shared_ptr<RenderObject>> mRenderList;
	std::vector<std::shared_ptr<RenderObject>> mTransparencyList;
	std::vector<std::shared_ptr<Light>>		   mLights;
	std::shared_ptr<Camera>					   mMainCamera;
	std::unique_ptr<UniformBuffer>			   mCameraBuffer;
	std::unique_ptr<UniformBuffer>			   mLightBuffer;
	std::unique_ptr<UniformBuffer> 			   mBoneMatrixBuffer;
	uint									   mLightCount;
	std::unique_ptr<ScreenQuad>				   mScreenQuad;
	Renderer*								   mRenderer;
	std::unique_ptr<Animator>		           mAnimator;
	std::vector<std::shared_ptr<Model>>        mModelList;
};