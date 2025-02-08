#include "pch.hpp"
#include "BasicRenderer.hpp"

#include "Materials/AlbedoNormalMaterial.hpp"
#include "Materials/ColorMaterial.hpp"

#include <functional>
#include <algorithm>

using namespace Resource;

void CreateMaterials(ResourceManager& resourceManager, Context::VulkanContext& context);
int ComputeFramePerSecond(float dt);

int main()
{
	srand(time(0));

	LOG_TRACE("Hello World!");

	Context::VulkanContext context;
	Context::Platform platform;

	Context::VulkanContextInfo contextInfo =
	{
		.appName = "App Example",
		.appVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
		.platform = &platform,
		.extensions =
		{
			.instanceExtensions =
			{},
			.instanceLayers =
			{}
		}
	};

	ECS::EntityComponentSystem ecs;

	platform.Initialize(contextInfo);
	context.Initialize(contextInfo);

	BasicRenderer renderer(&context);
	renderer.Build();

	ResourceManager resourceManager(context);
	resourceManager.RegisterResourceType<Mesh>();
	resourceManager.GetResourceType<Mesh>()->SetLoader(std::bind(&Mesh::LoadMesh, std::placeholders::_1, std::placeholders::_2));
	resourceManager.RegisterResourceType<Texture>();
	resourceManager.GetResourceType<Texture>()->SetLoader(std::bind(&Texture::LoadTexture, std::placeholders::_1, std::placeholders::_2));
	resourceManager.RegisterResourceType<Material>();
	resourceManager.RegisterResourceType<MaterialInstance>();

	CreateMaterials(resourceManager, context);

	resourceManager.Load<Mesh>("Barstool", "Models/Furniture/barstool.gltf");
	resourceManager.Load<Mesh>("Cube", "Models/Primitive/cube.fbx");
	resourceManager.Load<Mesh>("Debug Cube", "Models/Primitive/debugcube.fbx");

	resourceManager.Load<Texture>("Barstool Albedo", "Textures/Barstool/barstool_albedo.png");
	resourceManager.Load<Texture>("Barstool Normal", "Textures/Barstool/barstool_normal.png");

	//resourceManager.Load<Texture>("Wood Albedo", "Textures/Ground/wood_albedo.png");
	resourceManager.Load<Texture>("Wood Albedo", "Textures/Ground/blbl.png");
	resourceManager.Load<Texture>("Wood Normal", "Textures/Ground/wood_normal.png");

	resourceManager.Load<Texture>("Debug", "Textures/DebugCube/BaseMap.png");
	
	resourceManager.Add<MaterialInstance>("Barstool Material", 
		resourceManager.Get<Material>("AlbedoNormal")->CreateMaterialInstance<AlbedoNormalMaterial>(
			resourceManager.Get<Texture>("Barstool Albedo"), resourceManager.Get<Texture>("Barstool Normal")));


	resourceManager.Add<MaterialInstance>("Wood Material",
		resourceManager.Get<Material>("AlbedoNormal")->CreateMaterialInstance<AlbedoNormalMaterial>(
			resourceManager.Get<Texture>("Wood Albedo"), resourceManager.Get<Texture>("Wood Normal"), 100.f));

	resourceManager.Add<MaterialInstance>("Debug Material",
		resourceManager.Get<Material>("AlbedoNormal")->CreateMaterialInstance<AlbedoNormalMaterial>(
			resourceManager.Get<Texture>("Debug"), resourceManager.Get<Texture>("Wood Normal")));

	resourceManager.Add<MaterialInstance>("Light Gray",
		resourceManager.Get<Material>("Color")->CreateMaterialInstance<ColorMaterial>(
			glm::vec4{ 0.8f, 0.8f, 0.8f, 1.0f }));

	Util::Clock dtClock;

	Entity camera = ecs.CreateEntity();
	ecs.AddComponent<Transform>(camera, Transform({ 0, 5, -15 }));
	Camera cameraComponent;
	cameraComponent.far = 200.f;
	ecs.AddComponent<Camera>(camera, cameraComponent);

	Entity light = ecs.CreateEntity();
	DirectionalLight lightComponent;
	lightComponent.color = { 1.0f, 1.0f, 1.0f };
	lightComponent.intensity = 1.0f;
	lightComponent.direction = glm::normalize(glm::vec3{ 0.01f, -1.0f, 0.0f });
	lightComponent.cascadeCount = 4;
	lightComponent.shadowMapSize = 4096;
	ecs.AddComponent<DirectionalLight>(light, lightComponent);

	Entity player = ecs.CreateEntity();
	ecs.AddComponent<Transform>(player, Transform({ 0, 5, 0 }, { 1, 0, 0, 0 }, { .1f, .1f, .1f }));
	ecs.AddComponent<MeshRenderer>(player, MeshRenderer(resourceManager.Get<Mesh>("Barstool"), resourceManager.Get<MaterialInstance>("Barstool Material")));
	ecs.AddComponent<CharacterController>(player, CharacterController(10.f, 1.5f));
	ecs.AddComponent<CameraFollow>(player, CameraFollow(camera));

	Entity ground = ecs.CreateEntity();
	ecs.AddComponent<Transform>(ground, Transform({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, glm::vec3{ 350.0f, 1.f, 350.0f }));
	ecs.AddComponent<MeshRenderer>(ground, MeshRenderer(resourceManager.Get<Mesh>("Cube"), resourceManager.Get<MaterialInstance>("Wood Material")));

	for (int i = 0; i < 150; i++)
	{
		Entity debugcube = ecs.CreateEntity();
		ecs.AddComponent<Transform>(debugcube, Transform({ rand() % 200 - 100, rand() % 50 + 5, rand() % 200 - 100}, 
			glm::qua(glm::vec3(glm::radians(rand() / (float)RAND_MAX * 360.f), glm::radians(rand() / (float)RAND_MAX * 360.f), glm::radians(rand() / (float)RAND_MAX * 360.f))),
			glm::vec3{1.f, 1.f, 1.f}));
		ecs.AddComponent<MeshRenderer>(debugcube, MeshRenderer(resourceManager.Get<Mesh>("Debug Cube"), resourceManager.Get<MaterialInstance>("Debug Material")));
	}

	ecs.RegisterSystem<Controller>(context.GetPlatform()->GetWindow());
	ecs.RegisterSystem<BasicRenderSystem>(&renderer);

	while (!platform.ShouldClose())
	{
		float dt = dtClock.Restart();
		platform.PollEvents();
		ecs.Update(dt);
		platform.SetTitle(MF("FPS: ", ComputeFramePerSecond(dt)));
	}

	resourceManager.Cleanup();
	renderer.Cleanup();
	ecs.Cleanup();
	context.Shutdown();
	platform.CleanUp();
}

void CreateMaterials(ResourceManager& resourceManager, Context::VulkanContext& context)
{
	Material* colored = new Material(context.GetPipelinesManager()->GetPipeline({ "Colored" }), context.GetDescriptorSetLayoutsManager()->GetDescriptorSetLayout("Color"), &context);
	Material* textured = new Material(context.GetPipelinesManager()->GetPipeline({ "AlbedoNormal" }), context.GetDescriptorSetLayoutsManager()->GetDescriptorSetLayout("AlbedoNormal"), &context);

	resourceManager.Add<Material>("Color", colored);
	resourceManager.Add<Material>("AlbedoNormal", textured);
}

int ComputeFramePerSecond(float dt)
{
	static float currentUpdate = 0.0f;
	static float lastUpdate = 0.0f;
	static int frames = 0;
	static int fps = 0;

	currentUpdate += dt;
	frames++;

	if (currentUpdate - lastUpdate >= 1.0f)
	{
		fps = frames;
		frames = 0;
		lastUpdate = currentUpdate;
	}

	return fps;
}
