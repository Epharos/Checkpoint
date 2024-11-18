#include <Core.hpp>

int main()
{
	LOG_TRACE("Hello World!");

	Context::VulkanContext vulkanContext;
	vulkanContext.Initialize();

	vulkanContext.Shutdown();
}