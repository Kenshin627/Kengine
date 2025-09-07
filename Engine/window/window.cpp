#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "window.h"
#include "logger.h"
#include "graphic/renderer/renderer.h"

static void windowSizeChanged(GLFWwindow* window, int width, int height)
{
	Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
	if (!win)
	{
		assert(false);
		return;
	}	
	if (width == win->getWidth() && height == win->getHeight())
	{
		return;
	}
	win->setWindSize(width, height);	
}

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

	//event callbacks
	glfwSetWindowUserPointer(mWindow, this);
	glfwSetWindowSizeCallback(mWindow, windowSizeChanged);
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

void Window::setWidth(uint width)
{
	if (width != mWidth)
	{
		setWindSize(width, mHeight);
	}
}

void Window::setHeight(uint height)
{
	if (mHeight != height)
	{
		setWindSize(mWidth, height);
	}
}

void Window::setWindSize(uint width, uint height)
{
	if (width == mWidth && height == mHeight)
	{
		return;
	}
	onWindowSizeChanged(mWindow, width, height);
}

void Window::onWindowSizeChanged(GLFWwindow* window, int width, int height)
{
	mWidth = width;
	mHeight = height;
	//TODO: pass framebuffer
	//TODO: framebuffer resize 
	mRenderer->onWindowSizeChanged(mWidth, mHeight);
}
