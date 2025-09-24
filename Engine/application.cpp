#include <memory>
#include "window/window.h"
#include "application.h"
#include "scene/scene.h"
#include "scene/camera/camera.h"
#include "scene/light/pointLight/pointLight.h"
#include "scene/light/spotLight/spotLight.h"
#include "material/phongMaterial.h"
#include "material/pbrMaterial.h"
#include "scene/renderObject.h"
#include "scene/model/model.h"
#include "geometry/cube.h"
#include "geometry/rectangle.h"
#include "geometry/sphere.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "graphic/renderer/renderer.h"
#include "graphic/renderPass/postProcess/grayScaleEffect/grayScaleEffect.h"
#include "graphic/renderPass/deferredRendering/geometryPass.h"
#include "graphic/renderPass/deferredRendering/lightingPass.h"
#include "graphic/renderPass/ssaoPass/ssaoPass.h"
#include "graphic/renderPass/blurPass/blurPass.h"
#include "graphic/renderPass/bloomPass/bloomPass.h"
#include "graphic/renderPass/cascadeShadowMapPass/cascadeShadowMapPass.h"
#include "graphic/renderPass/gaussianBlur/gaussianBlur.h"
#include "graphic/renderPass/toneMapping/toneMapping.h"
#include "graphic/texture/textureSystem.h"

constexpr int shadowMapResolution = 4096;

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
	std::shared_ptr<Rectangle> groundGeom = std::make_shared<Rectangle>(100.0f, 100.0f, glm::vec2(0.1, 0.1));
	std::shared_ptr<Rectangle> wallGeom = std::make_shared<Rectangle>(100.0f, 100.0f, glm::vec2{0.5, 0.5});
	std::shared_ptr<Cube> cube = std::make_shared<Cube>(1.0f, 1.0f, 1.0f);
	std::shared_ptr<Sphere> sphereGeometry = std::make_shared<Sphere>(1.0f, 128.0f, 128.0f);

	//MATERIAL
	PBRMaterialSpecification groundSpec;
	groundSpec.albedoMapPath    = "images/black-white-tile-bl/black-white-tile_albedo.png";
	groundSpec.metallicMapPath  = "images/black-white-tile-bl/black-white-tile_metallic.png";
	groundSpec.roughnessMapPath = "images/black-white-tile-bl/black-white-tile_roughness.png";
	groundSpec.normalMapPath    = "images/black-white-tile-bl/black-white-tile_normal-ogl.png";
	std::shared_ptr<PBRMaterial> groundMat = std::make_shared<PBRMaterial>(groundSpec);

	PBRMaterialSpecification ground3Spec;
	ground3Spec.albedoMapPath = "images/rocky-rugged-terrain-bl/albedo.png";
	ground3Spec.roughnessMapPath = "images/rocky-rugged-terrain-bl/roughness.png";
	ground3Spec.normalMapPath = "images/rocky-rugged-terrain-bl/normal.png";
	ground3Spec.heightMapPath = "images/rocky-rugged-terrain-bl/height.png";
	std::shared_ptr<PBRMaterial> ground3Mat = std::make_shared<PBRMaterial>(ground3Spec);

	PBRMaterialSpecification wallSpec;
	wallSpec.albedoMapPath = "images/rectangle-polish/albedo.png";
	wallSpec.roughnessMapPath = "images/rectangle-polish/roughness.png";
	wallSpec.metallicMapPath = "images/rectangle-polish/metallic.png";
	wallSpec.normalMapPath = "images/rectangle-polish/normal.png";
	std::shared_ptr<PBRMaterial> wallMat = std::make_shared<PBRMaterial>(wallSpec);

	PBRMaterialSpecification wall2Spec;
	wall2Spec.albedoMapPath = "images/stonewall-bl/albedo.png";
	wall2Spec.roughnessMapPath = "images/stonewall-bl/roughness.png";
	wall2Spec.metallicMapPath = "images/stonewall-bl/metallic.png";
	wall2Spec.normalMapPath = "images/stonewall-bl/normal.png";
	wall2Spec.heightMapPath = "images/stonewall-bl/height.png";
	std::shared_ptr<PBRMaterial> wall2Mat = std::make_shared<PBRMaterial>(wall2Spec);

	PBRMaterialSpecification metallicSpec;
	metallicSpec.albedoMapPath = "images/worn-metal4-bl/worn_metal4_albedo.png";
	metallicSpec.metallicMapPath = "images/worn-metal4-bl/worn_metal4_Metallic.png";
	metallicSpec.roughnessMapPath = "images/worn-metal4-bl/worn_metal4_Roughness.png";
	metallicSpec.normalMapPath = "images/worn-metal4-bl/worn_metal4_Normal-ogl.png";
	std::shared_ptr<PBRMaterial> metallicMat = std::make_shared<PBRMaterial>(metallicSpec);

	PBRMaterialSpecification pbrSpec2;
	pbrSpec2.albedoMapPath = "images/space-cruiser-panels2-bl/albedo.png";
	pbrSpec2.metallicMapPath = "images/space-cruiser-panels2-bl/metallic.png";
	pbrSpec2.roughnessMapPath = "images/space-cruiser-panels2-bl/roughness.png";
	pbrSpec2.normalMapPath = "images/space-cruiser-panels2-bl/normal.png";
	pbrSpec2.heightMapPath = "images/space-cruiser-panels2-bl/height.png";
	std::shared_ptr<PBRMaterial> pbrMat2 = std::make_shared<PBRMaterial>(pbrSpec2);

	PBRMaterialSpecification pbrSpec3;
	pbrSpec3.roughness = 0.3;
	pbrSpec3.metallic = 0.8;
	pbrSpec3.albedoColor = { 0.3, 0.3, 0.3 };
	std::shared_ptr<PBRMaterial> pbrMat3 = std::make_shared<PBRMaterial>(pbrSpec3);

	PBRMaterialSpecification pbrSpec4;
	pbrSpec4.albedoMapPath = "images/patterned-bw-vinyl-bl/albedo.png";
	pbrSpec4.metallicMapPath = "images/patterned-bw-vinyl-bl/metallic.png";
	pbrSpec4.roughnessMapPath = "images/patterned-bw-vinyl-bl/roughness.png";
	pbrSpec4.normalMapPath = "images/patterned-bw-vinyl-bl/normal.png";
	std::shared_ptr<PBRMaterial> pbrMat4 = std::make_shared<PBRMaterial>(pbrSpec4);

	PBRMaterialSpecification pbrSpec5;
	pbrSpec5.albedoMapPath = "images/rounded-metal-cubes-bl/albedo.png";
	pbrSpec5.metallicMapPath = "images/rounded-metal-cubes-bl/metallic.png";
	pbrSpec5.roughnessMapPath = "images/rounded-metal-cubes-bl/roughness.png";
	pbrSpec5.normalMapPath = "images/rounded-metal-cubes-bl/normal.png";
	std::shared_ptr<PBRMaterial> pbrMat5 = std::make_shared<PBRMaterial>(pbrSpec5);

	PBRMaterialSpecification pbrSpec6;
	pbrSpec6.albedoMapPath = "images/peeling-painted-metal-bl/albedo.png";
	pbrSpec6.metallicMapPath = "images/peeling-painted-metal-bl/metallic.png";
	pbrSpec6.roughnessMapPath = "images/peeling-painted-metal-bl/roughness.png";
	pbrSpec6.normalMapPath = "images/peeling-painted-metal-bl/normal.png";
	std::shared_ptr<PBRMaterial> pbrMat6 = std::make_shared<PBRMaterial>(pbrSpec6);

	//RENDER OBJECT
	std::shared_ptr<RenderObject> ground = std::make_shared<RenderObject>("ground", groundGeom, ground3Mat);
	std::shared_ptr<RenderObject> wall = std::make_shared<RenderObject>("wall", wallGeom, wall2Mat);
	wall->setRotation(90, 0, 0);
	wall->setPosition(0, 0, -10);

	std::shared_ptr<RenderObject> sphere1 = std::make_shared<RenderObject>("sphere1", sphereGeometry, wall2Mat);
	sphere1->setPosition(0.6, 1.0, -0.8);
	std::shared_ptr<RenderObject> sphere2 = std::make_shared<RenderObject>("sphere2", sphereGeometry, pbrMat2);
	sphere2->setPosition(-2.8, 1.0, -3.6);
	std::shared_ptr<RenderObject> sphere3 = std::make_shared<RenderObject>("sphere3", sphereGeometry, pbrMat3);
	sphere3->setPosition(-6.4, 1.0, -1.2);
	std::shared_ptr<RenderObject> sphere4 = std::make_shared<RenderObject>("sphere4", sphereGeometry, pbrMat4);
	sphere4->setPosition(0.6, 1.0, 2.8);
	std::shared_ptr<RenderObject> sphere5 = std::make_shared<RenderObject>("sphere5", sphereGeometry, pbrMat5);
	sphere5->setPosition(-4.8, 1.0, 2.2);
	std::shared_ptr<RenderObject> sphere6 = std::make_shared<RenderObject>("sphere6", sphereGeometry, pbrMat6);
	sphere6->setPosition(-2.2, 1.0, 0.2);
	
	std::shared_ptr<RenderObject> box1 = std::make_shared<RenderObject>("box1", cube, pbrMat2);
	std::shared_ptr<RenderObject> box2 = std::make_shared<RenderObject>("box2", cube, metallicMat);
	box1->setPosition(-3, 0.5, -4);
	box2->setPosition(5, 0.5, -4);
	box2->setRotation(0, 25, 0);
	box2->setScale(0.5);
	scene->addRenderObject({ ground, wall, sphere1, sphere2, sphere3,sphere4, sphere5, sphere6, box1, box2 });
	
	//model
	/*Model model("models/backpack/backpack.obj");
	for (auto& renderObject : model.getRenderList())
	{
		renderObject->setPosition(2, 1, -3);
		renderObject->setScale(0.3);
	}*/
	//
	//Model model2("models/Cow.glb");
	//for (auto& renderObject : model2.getRenderList())
	//{
	//	renderObject->setPosition(2, 0, 2);
	//	renderObject->setRotation(0, 100, 0);
	//	renderObject->setScale(0.005);
	//}
	//
	//
	//Model model3("models/Large_troll.glb");
	//for (auto& renderObject : model3.getRenderList())
	//{
	//	renderObject->setPosition(-1, 0, 2);
	//	renderObject->setRotation(0, 20, 0);
	//	renderObject->setScale(0.3);
	//}
	//
	//Model model4("models/Labrador_retriever.glb");
	//for (auto& renderObject : model4.getRenderList())
	//{
	//	renderObject->setPosition(1, 0, 3);
	//	renderObject->setRotation(0, 60, 0);
	//	renderObject->setScale(0.2);
	//}
	
	//Oops.glb
	Model model5("models/Oops.glb");
	for (auto& renderObject : model5.getRenderList())
	{
		renderObject->setPosition(-1, 0.5, 0);
		renderObject->setScale(0.08);
	}

	
	//scene->addRenderObject(model.getRenderList());
	//scene->addRenderObject(model2.getRenderList());
	//scene->addRenderObject(model3.getRenderList());
	//scene->addRenderObject(model4.getRenderList());
	scene->addRenderObject(model5.getRenderList());
	
	//camera
	auto camera = std::make_shared<Camera>(glm::vec3(4, 6, 5), 54.0f, static_cast<float>(mWindow->getWidth()) / static_cast<float>(mWindow->getHeight()), 0.01f, 100);
	scene->setMainCamera(camera);

	//light
	auto light1 = std::make_shared<PointLight>("point Light1", glm::vec3(-0.5f, 1.7f, 0.0f), glm::vec3(500.0f/255.0f, 500.0f/255.0f, 1.0f), 1.0f, 0.09f, 0.032f);
	auto light2 = std::make_shared<PointLight>("point Light2", glm::vec3(0.4f, 1.5f, 1.0f), glm::vec3(0.0f/255.0f, 500.0f/255.0f, 0.0f/255.0f), 1.0f, 0.09f, 0.032f);
	auto light3 = std::make_shared<PointLight>("point Light3", glm::vec3(-2.3f, 1.0f, 2.0f), glm::vec3(100.0f/255.0f, 400.0f/255.0f, 400.0f/255.0f), 1.0f, 0.09f, 0.032f);
	auto spotLight1 = std::make_shared<SpotLight>("spot Light1", glm::vec3(3.0f, 2.0f, 2.0f), glm::vec3(-3.0f, -2.0f, -2.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.09f, 0.032f, 17.f, 15.5f);
	auto spotLight2 = std::make_shared<SpotLight>("spot Light2", glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(10.0f, 0.0f, 0.0f), 1.0f, 0.09f, 0.032f, 17.5f, 15.5f);
	auto spotLight3 = std::make_shared<SpotLight>("spot Light3", glm::vec3(0.0f, 2.0f, 10.0f), glm::vec3(0.0f, -2.0f, -10.0f), glm::vec3(5.0f, 5.0f, 5.0f), 1.0f, 0.09f, 0.032f, 17.5f, 15.5f);
	scene->addLights({ light1, light2, light3, spotLight1 });
	
	//pass
	//PASS GROUP#1
	glm::vec4 viewport{ 0, 0, width, height };

	//std::shared_ptr<DefaultPass> defaultPass = std::make_shared<DefaultPass>(RenderState{ viewport, RenderTarget::SCREEN });

	//std::shared_ptr<GrayScaleEffect> grayScalePass = std::make_shared<GrayScaleEffect>(RenderState{ viewport, RenderTarget::SCREEN, false });
	//grayScalePass->setLastPassFBOs({ defaultPass->getCurrentFrameBuffer().get()});

	//PASS GROUP #2
	//CSM
	//RenderState cascadeShadowMapPassState{ { 0, 0, shadowMapResolution, shadowMapResolution }, RenderTarget::FRAMEBUFFER, true, GL_LESS, GL_TRUE, true, GL_FRONT };
	//
	//CascadeShadowMapPassSpecification spec{ scene.get(), 2, 6, 0.9, FrustumSplitMethod::Pratical  };
	//std::shared_ptr<CascadeShadowMapPass> cascadeShadowMapPass = std::make_shared<CascadeShadowMapPass>(spec, cascadeShadowMapPassState);
	//
	////pass#1 geometryPass
	//std::shared_ptr<GeometryPass> gPass = std::make_shared<GeometryPass>(RenderState{viewport, RenderTarget::FRAMEBUFFER});
	//
	////pass#2 ssaoPass
	//std::shared_ptr<SSAOPass> ssaoPass = std::make_shared<SSAOPass>(128, 1.0, RenderState{viewport, RenderTarget::FRAMEBUFFER, false});
	//ssaoPass->setLastPassFBOs({ gPass->getCurrentFrameBuffer().get()});
	//
	////pass#3 blurPass
	//std::shared_ptr<BlurPass> blurPass = std::make_shared<BlurPass>(4, RenderState{viewport, RenderTarget::FRAMEBUFFER, false});
	//blurPass->setLastPassFBOs({ ssaoPass->getCurrentFrameBuffer().get() });
	//
	////pass#4 lightingPass
	//std::shared_ptr<LightingPass> lightingPass = std::make_shared<LightingPass>(RenderState{viewport, RenderTarget::FRAMEBUFFER, false});
	//lightingPass->setLastPassFBOs({ gPass->getCurrentFrameBuffer().get(), blurPass->getCurrentFrameBuffer().get(), cascadeShadowMapPass->getCurrentFrameBuffer().get()});
	//lightingPass->setCascadedShadowMapPass(cascadeShadowMapPass.get());
	//
	////pass#5 bloom
	//std::shared_ptr<BloomPass> bloomPass = std::make_shared<BloomPass>(RenderState{viewport, RenderTarget::FRAMEBUFFER, false});
	//bloomPass->setLastPassFBOs({ lightingPass->getCurrentFrameBuffer().get()});
	//
	//std::shared_ptr<GaussianBlur> gaussianBlur = std::make_shared<GaussianBlur>(6, RenderState{viewport, RenderTarget::FRAMEBUFFER, false});
	//gaussianBlur->setLastPassFBOs({ bloomPass->getCurrentFrameBuffer().get() });
	//
	//std::shared_ptr<ToneMapping> toneMappingPass = std::make_shared<ToneMapping>(1.0, RenderState{ viewport, RenderTarget::FRAMEBUFFER, false });
	//toneMappingPass->setLastPassFBOs({ gaussianBlur->getOutputFrameBuffer(), bloomPass->getCurrentFrameBuffer().get() });
	//spotLight1->castShadow(cascadeShadowMapPass.get());
	//renderer->setRenderPass({ cascadeShadowMapPass, gPass, ssaoPass, blurPass, lightingPass, bloomPass, gaussianBlur, toneMappingPass });
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
