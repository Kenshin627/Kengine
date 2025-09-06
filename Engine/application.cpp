#include <memory>
#include "application.h"
#include "window/window.h"
#include "scene/scene.h"
#include "geometry/cube.h"
#include "material/phongMaterial.h"
#include "graphic/renderer/renderer.h"
#include "scene/renderObject.h"
#include "scene/camera/camera.h"
#include "scene/light/pointLight/pointLight.h"
#include "geometry/rectangle.h"
#include "graphic/gpuBuffer/frameBuffer.h"

Application::Application(uint width, uint height, const char* title)
	:mWindow(std::make_unique<Window>(width, height, title))
{
	std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>();
	mWindow->attachRenderer(renderer);
	std::shared_ptr<Scene> scene = std::make_shared<Scene>();
	renderer->setCurrentScene(scene);
	renderer->setViewport({ 0, 0, width, height });
	std::shared_ptr<Rectangle> rectangle = std::make_shared<Rectangle>(100.0f, 100.0f);
	std::shared_ptr<Cube> cube = std::make_shared<Cube>(1.0f, 1.0f, 1.0f);
	std::shared_ptr<PhongMaterial> phongMat = std::make_shared<PhongMaterial>("images/dog.jpg", "images/led.png");
	std::shared_ptr<PhongMaterial> groundMat = std::make_shared<PhongMaterial>("images/grid3.jpg", "images/grid3.jpg");
	std::shared_ptr<RenderObject> ground = std::make_shared<RenderObject>(rectangle, groundMat);
	ground->setRotation(-90, 0, 0);
	std::shared_ptr<RenderObject> box1 = std::make_shared<RenderObject>(cube, phongMat);
	std::shared_ptr<RenderObject> box2 = std::make_shared<RenderObject>(cube, phongMat);
	box1->setRotation(0, 45, 0);
	box1->setPosition(0, 0.5, 0);

	box2->setRotation(0, 45, 0);
	box2->setPosition(2, 0.5, 0);
	box2->setScale(0.5);
	scene->addRenderObject({ ground, box1, box2 });

	//camera
	auto camera = std::make_shared<Camera>(glm::vec3(3, 3, 5), 45.0f, static_cast<float>(mWindow->getWidth()) / static_cast<float>(mWindow->getHeight()), 0.1f, 100.0f);
	scene->setMainCamera(camera);
	//lights
	auto light1 = std::make_shared<PointLight>(glm::vec3(0.0f, 7.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), 1.0f, 0.09f, 0.032f);
	auto light2 = std::make_shared<PointLight>(glm::vec3(3.0f, 7.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, 0.09f, 0.032f);
	auto light3 = std::make_shared<PointLight>(glm::vec3(-3.0f, 7.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), 1.0f, 0.09f, 0.032f);
	scene->addPointLights({ light1, light2 , light3 });

	//test Framebuffer
	std::initializer_list<FrameBufferSpecification> specs =
	{
		{
			AttachmentType::Color, 
			TextureInternalFormat::RGB8, 
			TextureDataFormat::RGB, 
			TextureWarpMode::CLAMP_TO_EDGE, 
			TextureWarpMode::CLAMP_TO_EDGE, 
			TextureFilter::NEAREST, 
			TextureFilter::NEAREST
		},
		{
			AttachmentType::Color,
			TextureInternalFormat::RGB8,
			TextureDataFormat::RGB,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureFilter::NEAREST,
			TextureFilter::NEAREST
		},
		{
			AttachmentType::Color,
			TextureInternalFormat::RGBA8,
			TextureDataFormat::RGBA,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureFilter::NEAREST,
			TextureFilter::NEAREST
		},
		{
			AttachmentType::DepthStencil,
			TextureInternalFormat::DEPTH24STENCIL8,
			TextureDataFormat::R,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureFilter::NEAREST,
			TextureFilter::NEAREST
		}
	};
	FrameBuffer fbo(width, height, specs);
}

Application::~Application()
{

}

void Application::Run()
{
	mWindow->RunLoop();
}

void Application::initScene()
{
	
}
