#pragma once
#include <memory>

class Scene;
//RENDERER INCLUDE PASSES
class Renderer
{
public:
	Renderer();
	~Renderer();
	void clear(float r, float g, float b, float a);
	void setViewport(int x, int y, int width, int height);
	void render();
	void setCurrentScene(std::shared_ptr<Scene> scene) { mCurrentScene = scene; }
private:
	int mViewport[4];
	std::shared_ptr<Scene> mCurrentScene;
};