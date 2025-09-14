#pragma once
#include <glad/glad.h>
#include <memory>
#include <glm.hpp>


class Scene;
class RenderPass;
//RENDERER INCLUDE PASSES
class Renderer
{
public:
	Renderer(uint width, uint height);
	~Renderer();	
	void render();
	void setCurrentScene(std::shared_ptr<Scene> scene) { mCurrentScene = scene; }
	Scene* getCurrentScene();
	void onWindowSizeChanged(uint width, uint height);
	void setRenderPass(const std::initializer_list<std::shared_ptr<RenderPass>>& passes);
	uint getLastFrameBufferTexture() const;
private:
	void setDefaultRenderPass();
private:	
	std::shared_ptr<Scene>					 mCurrentScene;
	std::vector<std::shared_ptr<RenderPass>> mRenderPasses;
	uint mWidth;
	uint mHeight;
};