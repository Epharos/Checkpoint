#include "pch.hpp"
#include "BasicRenderer.hpp"
#include "ECS/Systems/RenderSystem.hpp"
#include "ECS/Systems/MoveSystem.hpp"

#include <functional>

using namespace Resource;

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
	resourceManager.GetResourceType<Mesh>()->SetLoader(
		[&](const Context::VulkanContext& _context, const std::string& _path) { return Mesh::LoadMesh(_context, _path); });
	resourceManager.RegisterResourceType<Texture>();
	resourceManager.GetResourceType<Texture>()->SetLoader(
		[&](const Context::VulkanContext& _context, const std::string& _path) { return Texture::LoadTexture(_context, _path); });

	resourceManager.Load<Mesh>("Barstool", "Models/Barstool/barstool.gltf");
	resourceManager.Load<Mesh>("Cube", "Models/Cube/cube.fbx");

	resourceManager.Load<Texture>("Barstool Albedo", "Textures/Barstool/barstool_albedo.png");

	Util::Clock dtClock;

	for (int i = 0; i < 500; i++)
	{
		Entity cubeEntity = ecs.CreateEntity();

		bool isCube = rand() % 2 == 0;
		//LOG_DEBUG(MF("Model: ", isCube ? "Cube" : "Barstool"));

		const int m = 100;

		ecs.AddComponent<Transform>(cubeEntity, Transform({ rand() % (m * 2) - m, rand() % (m * 2) / 4 - m / 4, rand() % (m * 2) - m }, { 1.0f, 0.0f, 0.0f, 0.0f }, isCube ? glm::vec3{ 1.f, 1.f, 1.f } : glm::vec3{ .1f, .1f, .1f }));
		ecs.AddComponent<MeshRenderer>(cubeEntity, MeshRenderer(resourceManager.Get<Resource::Mesh>(isCube ? "Cube" : "Barstool")));

		/*LOG_DEBUG(MF("Spawning at: ", transform.GetPosition().x, ";", transform.GetPosition().y, ";", transform.GetPosition().z));*/
	}

	ecs.RegisterSystem<MoveSystem>();
	ecs.RegisterSystem<RenderSystem>(&renderer);

	while (!platform.ShouldClose())
	{
		platform.PollEvents();
		ecs.Update(dtClock.Restart());
	}

	resourceManager.Cleanup();
	renderer.Cleanup();
	context.Shutdown();
	platform.CleanUp();
}