#include <memory>
#include "window/window.h"
#include "application.h"
#include "scene/scene.h"
#include "scene/camera/camera.h"
#include "scene/light/pointLight/pointLight.h"
#include "scene/light/spotLight/spotLight.h"
#include "material/phongMaterial.h"
#include "scene/renderObject.h"
#include "scene/model/model.h"
#include "geometry/cube.h"
#include "geometry/rectangle.h"
#include "geometry/sphere.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "graphic/renderer/renderer.h"
#include "graphic/renderPass/defaultToScreen/defaultPass.h"
#include "graphic/renderPass/postProcess/grayScaleEffect/grayScaleEffect.h"
#include "graphic/renderPass/deferredRendering/geometryPass.h"
#include "graphic/renderPass/deferredRendering/lightingPass.h"
#include "graphic/renderPass/ssaoPass/ssaoPass.h"
#include "graphic/renderPass/blurPass/blurPass.h"
#include "graphic/renderPass/bloomPass/bloomPass.h"
#include "graphic/renderPass/gaussianBlur/gaussianBlur.h"
#include "graphic/renderPass/toneMapping/toneMapping.h"
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
	std::shared_ptr<Rectangle> groundGeom = std::make_shared<Rectangle>(100.0f, 100.0f);
	std::shared_ptr<Rectangle> wallGeom = std::make_shared<Rectangle>(100.0f, 100.0f);
	std::shared_ptr<Cube> cube = std::make_shared<Cube>(1.0f, 1.0f, 1.0f);
	std::shared_ptr<Sphere> sphereGeometry = std::make_shared<Sphere>(0.5f, 32.0f, 32.0f);

	//TEXTURE
	TextureSystem& ts = TextureSystem::getInstance();
	auto groundDiffuse = ts.getTexture("images/brick_crosswalk_4k.blend/textures/brick_crosswalk_diff_4k.jpg");
	auto groundSpecular = ts.getTexture("images/brick_crosswalk_4k.blend/textures/brick_crosswalk_disp_4k.png");
	auto groundNormal = ts.getTexture("images/brick_crosswalk_4k.blend/textures/brick_crosswalk_nor_gl_4k.png");
	auto groundShiness = ts.getTexture("images/brick_crosswalk_4k.blend/textures/brick_crosswalk_rough_4k.png");

	auto sphereDifuse = ts.getTexture("images/green_metal_rust_4k.blend/textures/green_metal_rust_diff_4k.jpg");
	auto sphereSpecular = ts.getTexture("images/green_metal_rust_4k.blend/textures/green_metal_rust_disp_4k.png");
	auto sphereNormal = ts.getTexture("images/green_metal_rust_4k.blend/textures/green_metal_rust_nor_gl_4k.jpg");
	auto sphereShiness = ts.getTexture("images/green_metal_rust_4k.blend/textures/green_metal_rust_rough_4k.jpg");
	
	auto wallDiffuse = ts.getTexture("images/patterned_brick_wall_03_4k.blend/textures/patterned_brick_wall_03_diff_4k.jpg");
	auto wallSpecular = ts.getTexture("images/patterned_brick_wall_03_4k.blend/textures/patterned_brick_wall_03_disp_4k.png");
	auto wallNormal = ts.getTexture("images/patterned_brick_wall_03_4k.blend/textures/patterned_brick_wall_03_nor_gl_4k.jpg");
	auto wallShiness = ts.getTexture("images/patterned_brick_wall_03_4k.blend/textures/patterned_brick_wall_03_rough_4k.jpg");

	auto boxDiffuse = ts.getTexture("images/wooden_garage_door_4k.blend/textures/wooden_garage_door_diff_4k.jpg");
	auto boxSpecular = ts.getTexture("images/wooden_garage_door_4k.blend/textures/wooden_garage_door_disp_4k.png");
	auto boxNormal = ts.getTexture("images/wooden_garage_door_4k.blend/textures/wooden_garage_door_nor_gl_4k.jpg");
	auto boxShiness = ts.getTexture("images/wooden_garage_door_4k.blend/textures/wooden_garage_door_rough_4k.jpg");
	//MATERIAL
	BlinnPhongMaterialSpecification groundSpec;
	groundSpec.diffuseMap = groundDiffuse;
	//groundSpec.specularMap = groundSpecular;
	groundSpec.normalMap = groundNormal;
	groundSpec.shinessMap = groundShiness;
	std::shared_ptr<PhongMaterial> groundMat = std::make_shared<PhongMaterial>(groundSpec);

	BlinnPhongMaterialSpecification sphereSpec;
	sphereSpec.diffuseMap = sphereDifuse;
	//sphereSpec.specularMap = sphereSpecular;
	sphereSpec.normalMap = sphereNormal;
	sphereSpec.shinessMap = sphereShiness;
	std::shared_ptr<PhongMaterial> sphereMat = std::make_shared<PhongMaterial>(sphereSpec);

	BlinnPhongMaterialSpecification boxSpec;
	boxSpec.diffuseMap = boxDiffuse;
	//boxSpec.specularMap = boxSpecular;
	boxSpec.normalMap = boxNormal;
	boxSpec.shinessMap = boxShiness;
	std::shared_ptr<PhongMaterial> boxMat = std::make_shared<PhongMaterial>(boxSpec);
	
	BlinnPhongMaterialSpecification wallSpec;
	wallSpec.diffuseMap = wallDiffuse;
	//wallSpec.specularMap = wallSpecular;
	wallSpec.normalMap = wallNormal;
	wallSpec.shinessMap = wallShiness;
	std::shared_ptr<PhongMaterial> wallMat = std::make_shared<PhongMaterial>(wallSpec);

	//RENDER OBJECT
	std::shared_ptr<RenderObject> ground = std::make_shared<RenderObject>(groundGeom, groundMat);	
	std::shared_ptr<RenderObject> wall = std::make_shared<RenderObject>(wallGeom, wallMat);
	std::shared_ptr<RenderObject> box1 = std::make_shared<RenderObject>(cube, boxMat);
	std::shared_ptr<RenderObject> box2 = std::make_shared<RenderObject>(cube, boxMat);
	std::shared_ptr<RenderObject> sphere = std::make_shared<RenderObject>(sphereGeometry, sphereMat);
	box1->setPosition(-3, 0.5, -4);
	box2->setPosition(5, 0.5, -4);
	box2->setRotation(0, 25, 0);
	box2->setScale(0.5);
	wall->setRotation(90, 0, 0);
	wall->setPosition(0, 0, -5);
	//sphere->setPosition(2, 1.0, -3);
	
	//model
	Model model("models/backpack/backpack.obj");
	for (auto& renderObject : model.getRenderList())
	{
		renderObject->setPosition(2, 1, -3);
		renderObject->setScale(0.3);
	}

	Model model2("models/Cow.glb");
	for (auto& renderObject : model2.getRenderList())
	{
		renderObject->setPosition(2, 0, 2);
		renderObject->setRotation(0, 100, 0);
		renderObject->setScale(0.005);
	}
	
	
	Model model3("models/Large_troll.glb");
	for (auto& renderObject : model3.getRenderList())
	{
		renderObject->setPosition(-1, 0, 2);
		renderObject->setRotation(0, 20, 0);
		renderObject->setScale(0.3);
	}

	Model model4("models/Labrador_retriever.glb");
	for (auto& renderObject : model4.getRenderList())
	{
		renderObject->setPosition(1, 0, 3);
		renderObject->setRotation(0, 60, 0);
		renderObject->setScale(0.2);
	}
	//Oops.glb
	Model model5("models/Oops.glb");
	for (auto& renderObject : model5.getRenderList())
	{
		renderObject->setPosition(-1, 0.5, 0);
		renderObject->setScale(0.08);
	}

	scene->addRenderObject({box1, box2, wall, ground});
	scene->addRenderObject(model.getRenderList());
	scene->addRenderObject(model2.getRenderList());
	scene->addRenderObject(model3.getRenderList());
	scene->addRenderObject(model4.getRenderList());
	scene->addRenderObject(model5.getRenderList());
	
	//camera
	auto camera = std::make_shared<Camera>(glm::vec3(3, 2, 6), 45.0f, static_cast<float>(mWindow->getWidth()) / static_cast<float>(mWindow->getHeight()), 0.1f, 100.0f);
	scene->setMainCamera(camera);

	//light
	auto light1 = std::make_shared<PointLight>(glm::vec3(-5.0f, 1.0f, -.5f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.09f, 0.032f);
	auto light2 = std::make_shared<PointLight>(glm::vec3(0.0f, 1.0f, -.5f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.09f, 0.032f);
	auto light3 = std::make_shared<PointLight>(glm::vec3(5.0f, 1.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.09f, 0.032f);
	auto spotLight1 = std::make_shared<SpotLight>(glm::vec3(2.0f, 3.0f, 2.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 10.0f), 1.0f, 0.09f, 0.032f, 17.f, 7.5f);
	auto spotLight2 = std::make_shared<SpotLight>(glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(10.0f, 0.0f, 0.0f), 1.0f, 0.09f, 0.032f, 17.5f, 7.5f);
	auto spotLight3 = std::make_shared<SpotLight>(glm::vec3(0.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(5.0f, 5.0f, 5.0f), 1.0f, 0.09f, 0.032f, 17.5f, 7.5f);
	scene->addLights({ spotLight1, spotLight2, spotLight3 });
	
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
	std::shared_ptr<SSAOPass> ssaoPass = std::make_shared<SSAOPass>(128, 1.0, ssaoPassState);
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
	lightingPassState.target = RenderTarget::FRAMEBUFFER;
	std::shared_ptr<LightingPass> lightingPass = std::make_shared<LightingPass>(lightingPassState);
	lightingPass->setLastPassFBOs({ gPass->getCurrentFrameBuffer(), blurPass->getCurrentFrameBuffer()});

	RenderState bloomPassState;
	bloomPassState.width = width;
	bloomPassState.height = height;
	bloomPassState.viewport.z = width;
	bloomPassState.viewport.w = height;
	bloomPassState.depthTest = false;
	bloomPassState.target = RenderTarget::FRAMEBUFFER;
	std::shared_ptr<BloomPass> bloomPass = std::make_shared<BloomPass>(bloomPassState);
	bloomPass->setLastPassFBOs({ lightingPass->getCurrentFrameBuffer() });

	RenderState gaussianPassState;
	gaussianPassState.width = width;
	gaussianPassState.height = height;
	gaussianPassState.viewport.z = width;
	gaussianPassState.viewport.w = height;
	gaussianPassState.depthTest = false;
	gaussianPassState.target = RenderTarget::FRAMEBUFFER;
	std::shared_ptr<GaussianBlur> gaussianBlur = std::make_shared<GaussianBlur>(gaussianPassState);
	gaussianBlur->setLastPassFBOs({ bloomPass->getCurrentFrameBuffer() });

	RenderState toneMappingState;
	toneMappingState.width = width;
	toneMappingState.height = height;
	toneMappingState.viewport.z = width;
	toneMappingState.viewport.w = height;
	toneMappingState.depthTest = false;
	toneMappingState.target = RenderTarget::FRAMEBUFFER;
	std::shared_ptr<ToneMapping> toneMappingPass = std::make_shared<ToneMapping>(1.0, toneMappingState);
	toneMappingPass->setLastPassFBOs({ gaussianBlur->getOutputFrameBuffer(), bloomPass->getCurrentFrameBuffer() });

	renderer->setRenderPass({ gPass, ssaoPass, blurPass, lightingPass, bloomPass, gaussianBlur, toneMappingPass });
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
