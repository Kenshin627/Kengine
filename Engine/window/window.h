#pragma once
#include <glm.hpp>
#include "typedef.h"

class Renderer;
struct GLFWwindow;
class Window
{
public:
	Window(uint width, uint height, const char* title);
	~Window();
	void RunLoop();
	void attachRenderer(std::shared_ptr<Renderer> renderer);
	uint getWidth() const { return mWidth; }
	uint getHeight() const { return mHeight; }
	void setWidth(uint width);
	void setHeight(uint height);
	void setWindSize(uint width, uint height);
private:
	void onWindowSizeChanged(GLFWwindow* window, int width, int height);
	void onViewportSizeChanged(int width, int height);
private:
	uint					  mWidth;
	uint					  mHeight;
	const char*				  mTitle;
	GLFWwindow*				  mWindow  { nullptr };
	std::shared_ptr<Renderer> mRenderer;
	glm::vec2			      mImGuiViewportSize;
};
