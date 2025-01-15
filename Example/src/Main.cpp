#include "pch.hpp"
#include "BasicRenderer.hpp"
#include "ECS/Systems/RenderSystem.hpp"
#include "ECS/Systems/MoveSystem.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace Resource;

Mesh* LoadMesh(const std::string& _path);

int main()
{
	LOG_TRACE("Hello World!");

	Context::VulkanContext vulkanContext;
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
	vulkanContext.Initialize(contextInfo);

	BasicRenderer renderer;
	renderer.Build(&vulkanContext);

	ResourceManager resourceManager;
	resourceManager.RegisterResourceType<Mesh>();


#pragma region Cube Mesh TMP
	std::vector<Resource::Vertex> cubeVertices;
	std::vector<uint32_t> cubeIndices;

	cubeVertices.push_back({ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } });
	cubeVertices.push_back({ { 0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } });
	cubeVertices.push_back({ { 0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } });
	cubeVertices.push_back({ { -0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f } });

	cubeVertices.push_back({ { -0.5f, -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } });
	cubeVertices.push_back({ { 0.5f, -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } });
	cubeVertices.push_back({ { 0.5f, 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } });
	cubeVertices.push_back({ { -0.5f, 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } });

	cubeVertices.push_back({ { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } });
	cubeVertices.push_back({ { 0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } });
	cubeVertices.push_back({ { 0.5f, -0.5f, 0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } });
	cubeVertices.push_back({ { -0.5f, -0.5f, 0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } });

	cubeVertices.push_back({ { -0.5f, 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } });
	cubeVertices.push_back({ { 0.5f, 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } });
	cubeVertices.push_back({ { 0.5f, 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } });
	cubeVertices.push_back({ { -0.5f, 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } });

	cubeVertices.push_back({ { -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } });
	cubeVertices.push_back({ { -0.5f, -0.5f, 0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } });
	cubeVertices.push_back({ { -0.5f, 0.5f, 0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } });
	cubeVertices.push_back({ { -0.5f, 0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } });

	cubeVertices.push_back({ { 0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } });
	cubeVertices.push_back({ { 0.5f, -0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } });
	cubeVertices.push_back({ { 0.5f, 0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } });
	cubeVertices.push_back({ { 0.5f, 0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } });

	cubeIndices =
	{
		0, 1, 2, 2, 3, 0,
		4, 6, 5, 6, 4, 7,
		8, 10, 9, 10, 8, 11,
		12, 13, 14, 14, 15, 12,
		16, 18, 17, 18, 16, 19,
		20, 21, 22, 22, 23, 20
	};

	Mesh* cube = new Mesh(vulkanContext, cubeVertices, cubeIndices);
	resourceManager.Add<Mesh>("cube", cube);
	//resourceManager.Load<Mesh>("test");

#pragma endregion

	Util::Clock dtClock;

	for (int i = 0; i < 500; i++)
	{
		Entity cubeEntity = ecs.CreateEntity();
		ecs.AddComponent<Transform>(cubeEntity, Transform({ rand() % 100 - 50, rand() % 100 - 50, rand() % 100 - 50 }, {1.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}));
		ecs.AddComponent<MeshRenderer>(cubeEntity, MeshRenderer(resourceManager.Get<Resource::Mesh>("cube")));
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
	vulkanContext.Shutdown();
	platform.CleanUp();
}

Mesh* LoadMesh(const std::string& _path)
{
	return nullptr;
}