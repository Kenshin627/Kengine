#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "window.h"
#include "logger.h"
#include "graphic/renderer/renderer.h"

Window::Window(uint width, uint height, const char* title)
	:mWidth(width),
	 mHeight(height),
	 mTitle(title),
	 mWindow(nullptr)
{
	if (!glfwInit())
	{
		KS_CORE_ERROR("Failed to initialize GLFW");
		return;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	mWindow = glfwCreateWindow(mWidth, mHeight, mTitle, nullptr, nullptr);
	if (!mWindow)
	{
		KS_CORE_ERROR("Failed to create GLFW window");
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(mWindow);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		KS_CORE_ERROR("Failed to initialize GLAD");
		return;
	}

}

Window::~Window()
{
	glfwTerminate();
}

void Window::RunLoop()
{
	while (!glfwWindowShouldClose(mWindow))
	{
		if (mRenderer)
		{
			mRenderer->clear(0.2, 0.2, 0.2, 1.0);
			mRenderer->setViewport(0, 0, mWidth, mHeight);
			mRenderer->render();
		}
		glfwPollEvents();
		glfwSwapBuffers(mWindow);
	}
	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

void Window::attachRenderer(std::shared_ptr<Renderer> renderer)
{
	if (mRenderer != renderer)
	{
		mRenderer = renderer;
	}
}
