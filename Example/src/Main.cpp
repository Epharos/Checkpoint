#include <Core.hpp>
#include "BasicRenderer.hpp"

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

	platform.Initialize(contextInfo);
	vulkanContext.Initialize(contextInfo);

	BasicRenderer renderer;
	renderer.Build(&vulkanContext);

	while (!platform.ShouldClose())
	{
		platform.PollEvents();
		renderer.Render();
	}

	renderer.Cleanup();
	vulkanContext.Shutdown();
	platform.CleanUp();
}