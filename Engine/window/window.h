#pragma once
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
private:
	uint					  mWidth;
	uint					  mHeight;
	const char*				  mTitle;
	GLFWwindow*				  mWindow  { nullptr };
	std::shared_ptr<Renderer> mRenderer;
};
