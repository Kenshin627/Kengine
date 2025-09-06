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

Application::Application(uint width, uint height, const char* title)
	:mWindow(std::make_unique<Window>(width, height, title))
{
	std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>();
	mWindow->attachRenderer(renderer);
	std::shared_ptr<Scene> scene = std::make_shared<Scene>();
	renderer->setCurrentScene(scene);
	std::shared_ptr<Rectangle> rectangle = std::make_shared<Rectangle>(1.0f, 1.0f);
	std::shared_ptr<Cube> cube = std::make_shared<Cube>(1.0f, 1.0f, 1.0);
	std::shared_ptr<PhongMaterial> phongMat = std::make_shared<PhongMaterial>("images/led.png", "images/led.png");
	std::shared_ptr<RenderObject> obj1 = std::make_shared<RenderObject>(rectangle, phongMat);
	std::shared_ptr<RenderObject> obj2 = std::make_shared<RenderObject>(cube, phongMat);
	obj2->setRotation(0, 45, 0);
	scene->addRenderObject({obj2});

	//camera
	auto camera = std::make_shared<Camera>(glm::vec3(0, 1, 5), 45.0f, static_cast<float>(mWindow->getWidth()) / static_cast<float>(mWindow->getHeight()), 0.1f, 100.0f);
	scene->setMainCamera(camera);
	//lights
	auto light = std::make_shared<PointLight>(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.09f, 0.032f);
	scene->addPointLight(light);
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
