#pragma once
#include <glad/glad.h>
#include <memory>
#include <glm.hpp>

class Scene;
//RENDERER INCLUDE PASSES
class Renderer
{
public:
	Renderer();
	~Renderer();
	void clear();
	void setViewport(const glm::vec4& viewport);
	void viewPort() const;
	void setClearColor(const glm::vec4& c);
	void setClearColor(float r, float g, float b, float a);
	void setClearDepth(double d);
	void render();
	void setCurrentScene(std::shared_ptr<Scene> scene) { mCurrentScene = scene; }
	void onWindowSizeChanged(uint width, uint height);
private:
	glm::vec4			   mViewport		{ 0.0, 0.0, 0.0, 0.0 };
	glm::vec4			   mClearColor		{ 0.3, 0.3, 0.3, 1.0 };
	double				   mClearDepth		{ 1.0 };
	std::shared_ptr<Scene> mCurrentScene;
	uint				   mClearBits		{ GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT };
};