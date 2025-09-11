#include <memory>
#include "application.h"
#include "window/window.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "scene/scene.h"
#include "scene/camera/camera.h"
#include "scene/light/pointLight/pointLight.h"
#include "scene/light/spotLight/spotLight.h"
#include "scene/model/model.h"

#include "material/phongMaterial.h"

#include "scene/renderObject.h"
#include "geometry/cube.h"
#include "geometry/rectangle.h"
#include "geometry/sphere.h"

#include "graphic/renderer/renderer.h"
#include "graphic/renderPass/defaultToScreen/defaultPass.h"
#include "graphic/renderPass/postProcess/grayScaleEffect/grayScaleEffect.h"
#include "graphic/renderPass/deferredRendering/geometryPass.h"
#include "graphic/renderPass/deferredRendering/lightingPass.h"
#include "graphic/renderPass/ssaoPass/ssaoPass.h"
#include "graphic/renderPass/blurPass/blurPass.h"
#include "graphic/texture/textureSystem.h"

Application::Application(uint width, uint height, const char* title)
	:mWindow(std::make_unique<Window>(width, height, title))
{
	//renderer
	std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>(width, height);
	mWindow->attachRenderer(renderer);

	////////SCENE//////////////////////////////////////////////

	//GEOMETRY
	std::shared_ptr<Scene> scene = std::make_shared<Scene>();
	renderer->setCurrentScene(scene);
	std::shared_ptr<Rectangle> rectangle = std::make_shared<Rectangle>(100.0f, 100.0f);
	std::shared_ptr<Cube> cube = std::make_shared<Cube>(1.0f, 1.0f, 1.0f);
	std::shared_ptr<Sphere> sphereGeometry = std::make_shared<Sphere>(1.0f, 32.0f, 32.0f);

	//TEXTURE
	TextureSystem& ts = TextureSystem::getInstance();
	auto woodenDiffuse = ts.getTexture("images/wooden1.jpg");

	auto greyDiffuse = ts.getTexture("images/greyDiff.jpg");

	auto groundDiffuse = ts.getTexture("images/groundSpec2.jpg");

	//MATERIAL
	BlinnPhongMaterialSpecification spec1;
	spec1.diffuseMap = woodenDiffuse;
	std::shared_ptr<PhongMaterial> wooden = std::make_shared<PhongMaterial>(spec1);

	BlinnPhongMaterialSpecification spec2;
	spec2.diffuseMap = greyDiffuse;
	std::shared_ptr<PhongMaterial> grey = std::make_shared<PhongMaterial>(spec2);

	BlinnPhongMaterialSpecification spec3;
	spec3.diffuseMap = groundDiffuse;
	std::shared_ptr<PhongMaterial> groundMat = std::make_shared<PhongMaterial>(spec3);
	
	//RENDER OBJECT
	std::shared_ptr<RenderObject> ground = std::make_shared<RenderObject>(rectangle, groundMat);
	
	std::shared_ptr<RenderObject> box1 = std::make_shared<RenderObject>(cube, wooden);
	std::shared_ptr<RenderObject> box2 = std::make_shared<RenderObject>(cube, wooden);
	std::shared_ptr<RenderObject> sphere = std::make_shared<RenderObject>(sphereGeometry, grey);
	//box1->setRotation(0, 45, 0);
	box1->setPosition(0, 0.5, 0);
	//box2->setRotation(0, 45, 0);
	box2->setPosition(2, 0.5, 0);
	box2->setScale(0.5);
	sphere->setPosition(-1, 1.0, 2);
	
	//model
	//Model model("models/backpack/backpack.obj");

	scene->addRenderObject({ ground, box1, box2, sphere });
	//scene->addRenderObject(model.getRenderList());
	//camera
	auto camera = std::make_shared<Camera>(glm::vec3(3, 3, 5), 45.0f, static_cast<float>(mWindow->getWidth()) / static_cast<float>(mWindow->getHeight()), 0.1f, 100.0f);
	scene->setMainCamera(camera);

	//light
	auto light1 = std::make_shared<PointLight>(glm::vec3(0.0f, 4.0f, 0.f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.09f, 0.032f);
	auto light2 = std::make_shared<PointLight>(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.09f, 0.032f);
	auto light3 = std::make_shared<PointLight>(glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.09f, 0.032f);
	auto spotLight1 = std::make_shared<SpotLight>(glm::vec3(2.0f, 5.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 1.0f), 1.0f, 0.09f, 0.032f, 35.f, 5.f);
	auto spotLight2 = std::make_shared<SpotLight>(glm::vec3(-2.0f, 5.0f, 2.0f), glm::vec3(-0.0f, -1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 1.0f), 1.0f, 0.09f, 0.032f, 35.f, 5.f);
	scene->addLights({ light1, spotLight1, spotLight2 });
	
	//pass
	//PASS GROUP#1
	//pass#1 to framebuffer
	RenderState defaultPassState;
	defaultPassState.width = width;
	defaultPassState.height = height;
	defaultPassState.viewport.z = width;
	defaultPassState.viewport.w = height;
	defaultPassState.target = RenderTarget::SCREEN;
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
	grayScalePass->setLastPassFBOs({ defaultPass->getCurrentFrameBuffer() });

	//PASS GROUP #2
	//pass#1 geometryPass
	RenderState geometryPassState;
	geometryPassState.width = width;
	geometryPassState.height = height;
	geometryPassState.viewport.z = width;
	geometryPassState.viewport.w = height;
	geometryPassState.target = RenderTarget::FRAMEBUFFER;
	std::shared_ptr<GeometryPass> gPass = std::make_shared<GeometryPass>(geometryPassState);

	//pass#2 ssaoPass
	RenderState ssaoPassState;
	ssaoPassState.width = width;
	ssaoPassState.height = height;
	ssaoPassState.viewport.z = width;
	ssaoPassState.viewport.w = height;
	ssaoPassState.depthTest = false;
	ssaoPassState.target = RenderTarget::FRAMEBUFFER;
	std::shared_ptr<SSAOPass> ssaoPass = std::make_shared<SSAOPass>(128, 2.0, ssaoPassState);
	ssaoPass->setLastPassFBOs({ gPass->getCurrentFrameBuffer() });

	//pass#3 blurPass
	RenderState blurPasState;
	blurPasState.width = width;
	blurPasState.height = height;
	blurPasState.viewport.z = width;
	blurPasState.viewport.w = height;
	blurPasState.depthTest = false;
	blurPasState.target = RenderTarget::FRAMEBUFFER;
	std::shared_ptr<BlurPass> blurPass = std::make_shared<BlurPass>(4, blurPasState);
	blurPass->setLastPassFBOs({ ssaoPass->getCurrentFrameBuffer() });
	//pass#4 lightingPass
	RenderState lightingPassState;
	lightingPassState.width = width;
	lightingPassState.height = height;
	lightingPassState.viewport.z = width;
	lightingPassState.viewport.w = height;
	lightingPassState.depthTest = false;
	lightingPassState.target = RenderTarget::SCREEN;
	std::shared_ptr<LightingPass> lightingPass = std::make_shared<LightingPass>(lightingPassState);

	lightingPass->setLastPassFBOs({ gPass->getCurrentFrameBuffer(), blurPass->getCurrentFrameBuffer()});

	renderer->setRenderPass({ gPass, ssaoPass, blurPass, lightingPass });
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
