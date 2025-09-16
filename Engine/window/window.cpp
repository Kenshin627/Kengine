#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "window.h"
#include "logger.h"
#include "graphic/renderer/renderer.h"
#include "scene/light/light.h"
#include "scene/scene.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "scene/light/spotLight/spotLight.h"

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
	//IMGUI////////////////////
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// 启用键盘导航
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // 核心：启用docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // 可选：支持多窗口系统
	io.ConfigDockingWithShift = false;                          // 无需Shift键即可停靠
	io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/Candara.ttf", 22.0f);
	// 5. 配置样式（可选）
	ImGui::StyleColorsDark(); // 暗色主题

	// 6. 初始化平台和渲染后端
	ImGui_ImplGlfw_InitForOpenGL(mWindow, true);
	ImGui_ImplOpenGL3_Init("#version 460 core");

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
		glfwPollEvents();
		//IMGUI//////////////////////
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		//docking//////
		// 5. 创建Dock Space（关键步骤）
		// 覆盖整个窗口的dock空间
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		// 窗口标志：无边框、无标题栏，作为dock空间容器
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		// 创建dock空间容器窗口
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::Begin("DockSpace Container", nullptr, window_flags);
		ImGui::PopStyleVar(2);

		// 生成dock空间
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
		}
		ImGui::End();
		
		//check imgui dock viewport windowsize change, then resize fbo and camera aspect ratio, then draw to fbo, blit to imgui image
		ImGui::Begin("Viewport");
		ImVec2 viewportSize = ImGui::GetWindowSize();
		onViewportSizeChanged(viewportSize.x, viewportSize.y);
		if (mRenderer)
		{
			mRenderer->render();
		}
		ImGui::Image((void*)(intptr_t)mRenderer->getLastFrameBufferTexture(), ImVec2(viewportSize.x, viewportSize.y), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End();

		auto scene = mRenderer->getCurrentScene();
		scene->updateSceneUI();

		ImGui::Render();
		glClearColor(0.1, 0.1, 0.1, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		// 渲染ImGui绘制数据
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// 多视口支持（如果启用）
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
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
	//最小化判断
	if (width == 0 || height == 0)
	{
		return;
	}
	mWidth = width;
	mHeight = height;
	mRenderer->onWindowSizeChanged(mWidth, mHeight);
}

void Window::onViewportSizeChanged(int width, int height)
{
	if (width > 0 && height > 0)
	{
		if (mImGuiViewportSize.x == width && mImGuiViewportSize.y == height)
		{
			return;
		}
		mImGuiViewportSize.x = width;
		mImGuiViewportSize.y = height;
		mRenderer->onWindowSizeChanged(width, height);
	}
}
