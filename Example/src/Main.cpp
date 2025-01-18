#include "pch.hpp"
#include "BasicRenderer.hpp"
#include "ECS/Systems/RenderSystem.hpp"
#include "ECS/Systems/MoveSystem.hpp"

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

	BasicRenderer renderer;
	renderer.Build(&context);

	ResourceManager resourceManager(context);
	resourceManager.RegisterResourceType<Mesh>();
	resourceManager.GetResourceType<Mesh>()->SetLoader(std::bind(&Mesh::LoadMesh, std::placeholders::_1, std::placeholders::_2));
	resourceManager.RegisterResourceType<Texture>();
	resourceManager.GetResourceType<Texture>()->SetLoader(std::bind(&Texture::LoadTexture, std::placeholders::_1, std::placeholders::_2));
	resourceManager.RegisterResourceType<Material>();
	resourceManager.RegisterResourceType<MaterialInstance>();

	CreateMaterials(resourceManager, context);

	resourceManager.Load<Mesh>("Barstool", "Models/Barstool/barstool.gltf");
	resourceManager.Load<Mesh>("Cube", "Models/Cube/cube.fbx");

	resourceManager.Load<Texture>("Barstool Albedo", "Textures/Barstool/barstool_albedo.png");
	resourceManager.Load<Texture>("Barstool Normal", "Textures/Barstool/barstool_normal.png");
	
	resourceManager.Add<MaterialInstance>("Barstool Material", 
		resourceManager.Get<Material>("AlbedoNormal")->CreateMaterialInstance<AlbedoNormalMaterial>(
			resourceManager.Get<Texture>("Barstool Albedo"), resourceManager.Get<Texture>("Barstool Normal")));

	resourceManager.Add<MaterialInstance>("Blue",
		resourceManager.Get<Material>("Color")->CreateMaterialInstance<ColorMaterial>(
			glm::vec4{ 0.0f, 0.0f, 1.0f, 1.0f }));

	resourceManager.Add<MaterialInstance>("Yellow",
		resourceManager.Get<Material>("Color")->CreateMaterialInstance<ColorMaterial>(
			glm::vec4{ 1.0f, 1.0f, 0.0f, 1.0f }));

	Util::Clock dtClock;

	const int m = 200;

	for (int i = 0; i < 200; i++)
	{
		Entity entity = ecs.CreateEntity();

		ecs.AddComponent<Transform>(entity, Transform({ rand() % (m * 2) - m, rand() % (m * 2) / 4 - m / 4, rand() % (m * 2) - m }, { 1.0f, 0.0f, 0.0f, 0.0f }, glm::vec3{ .1f, .1f, .1f }));
		ecs.AddComponent<MeshRenderer>(entity, MeshRenderer(resourceManager.Get<Mesh>("Barstool"), resourceManager.Get<MaterialInstance>(rand() % 2 ? "Blue" : "Yellow")));
	}

	for (int i = 0; i < 150; i++)
	{
		Entity entity = ecs.CreateEntity();

		ecs.AddComponent<Transform>(entity, Transform({ rand() % (m * 2) - m, rand() % (m * 2) / 4 - m / 4, rand() % (m * 2) - m }, { 1.0f, 0.0f, 0.0f, 0.0f }, glm::vec3{ .9f, .9f, .9f }));
		ecs.AddComponent<MeshRenderer>(entity, MeshRenderer(resourceManager.Get<Mesh>("Cube"), resourceManager.Get<MaterialInstance>(rand() % 2 ? "Blue" : "Yellow")));
	}

	for (int i = 0; i < 350; i++)
	{
		Entity entity = ecs.CreateEntity();

		ecs.AddComponent<Transform>(entity, Transform({ rand() % (m * 2) - m, rand() % (m * 2) / 4 - m / 4, rand() % (m * 2) - m }, { 1.0f, 0.0f, 0.0f, 0.0f }, glm::vec3{ .2f, .2f, .2f }));
		ecs.AddComponent<MeshRenderer>(entity, MeshRenderer(resourceManager.Get<Mesh>("Barstool"), resourceManager.Get<MaterialInstance>("Barstool Material")));
	}

	ecs.RegisterSystem<MoveSystem>();
	ecs.RegisterSystem<RenderSystem>(&renderer);

	while (!platform.ShouldClose())
	{
		float dt = dtClock.Restart();
		platform.PollEvents();
		ecs.Update(dt);
		platform.SetTitle(MF("FPS: ", ComputeFramePerSecond(dt)));
	}

	resourceManager.Cleanup();
	renderer.Cleanup();
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
