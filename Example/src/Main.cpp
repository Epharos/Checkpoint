#include <Core.hpp>

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

	while (!platform.ShouldClose())
	{
		platform.PollEvents();
	}

	vulkanContext.Shutdown();
	platform.CleanUp();
}