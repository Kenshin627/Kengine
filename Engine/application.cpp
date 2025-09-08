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
#include "graphic/renderPass/defaultToScreen/defaultPass.h"
#include "graphic/renderPass/postProcess/grayScaleEffect/grayScaleEffect.h"
#include "graphic/renderPass/deferredRendering/geometryPass.h"
#include "graphic/renderPass/deferredRendering/lightingPass.h"
#include "graphic/renderPass/ssaoPass/ssaoPass.h"

Application::Application(uint width, uint height, const char* title)
	:mWindow(std::make_unique<Window>(width, height, title))
{
	//renderer
	std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>(width, height);
	mWindow->attachRenderer(renderer);

	//scene
	std::shared_ptr<Scene> scene = std::make_shared<Scene>();
	renderer->setCurrentScene(scene);
	std::shared_ptr<Rectangle> rectangle = std::make_shared<Rectangle>(100.0f, 100.0f);
	std::shared_ptr<Cube> cube = std::make_shared<Cube>(1.0f, 1.0f, 1.0f);
	std::shared_ptr<PhongMaterial> phongMat = std::make_shared<PhongMaterial>("images/dog.jpg", "images/dog.jpg", 128);
	std::shared_ptr<PhongMaterial> groundMat = std::make_shared<PhongMaterial>("images/grid3.jpg", "images/grid3.jpg", 128);
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

	//light
	auto light1 = std::make_shared<PointLight>(glm::vec3(0.0f, 6.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.09f, 0.032f);
	auto light2 = std::make_shared<PointLight>(glm::vec3(3.0f, 6.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.09f, 0.032f);
	auto light3 = std::make_shared<PointLight>(glm::vec3(-3.0f, 6.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.09f, 0.032f);
	scene->addPointLights({ light1, light2 , light3 });
	
	//pass
	//PASS GROUP#1
	//pass#1 to framebuffer
	RenderState defaultPassState;
	defaultPassState.width = width;
	defaultPassState.height = height;
	defaultPassState.viewport.z = width;
	defaultPassState.viewport.w = height;
	defaultPassState.target = RenderTarget::FRAMEBUFFER;
	std::shared_ptr<DefaultPass> defaultPass = std::make_shared<DefaultPass>(defaultPassState);

	//pass#2 graySCcale effect
	RenderState grayScalePassState;
	grayScalePassState.width = width;
	grayScalePassState.height = height;
	grayScalePassState.viewport.z = width;
	grayScalePassState.viewport.w = height;
	grayScalePassState.target = RenderTarget::SCREEN;
	grayScalePassState.depthTest = false;
	std::shared_ptr<GrayScaleEffect> grayScalePass = std::make_shared<GrayScaleEffect>(grayScalePassState);
	grayScalePass->setLastPassFBO(defaultPass->getCurrentFrameBuffer());

	//PASS GROUP#2
	//pass#1 geometryPass
	RenderState geometryPassState;
	geometryPassState.width = width;
	geometryPassState.height = height;
	geometryPassState.viewport.z = width;
	geometryPassState.viewport.w = height;
	geometryPassState.target = RenderTarget::FRAMEBUFFER;
	std::shared_ptr<GeometryPass> gPass = std::make_shared<GeometryPass>(geometryPassState);

	//pass#2 lightingPass
	RenderState lightingPassState;
	lightingPassState.width = width;
	lightingPassState.height = height;
	lightingPassState.viewport.z = width;
	lightingPassState.viewport.w = height;
	lightingPassState.depthTest = false;
	lightingPassState.target = RenderTarget::SCREEN;
	std::shared_ptr<LightingPass> lightingPass = std::make_shared<LightingPass>(lightingPassState);
	lightingPass->setLastPassFBO(gPass->getCurrentFrameBuffer());

	//PASS GROUP#3
	//pass#1 geometryPass
	//pass#2 ssaoPass
	RenderState ssaoPassState;
	ssaoPassState.width = width;
	ssaoPassState.height = height;
	ssaoPassState.viewport.z = width;
	ssaoPassState.viewport.w = height;
	ssaoPassState.depthTest = false;
	ssaoPassState.target = RenderTarget::SCREEN;
	std::shared_ptr<SSAOPass> ssaoPass = std::make_shared<SSAOPass>(64, 1, ssaoPassState);
	ssaoPass->setLastPassFBO(gPass->getCurrentFrameBuffer());
	renderer->setRenderPass({ gPass, ssaoPass });
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
