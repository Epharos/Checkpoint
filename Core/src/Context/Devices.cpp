#include "pch.hpp"
#include "Devices.hpp"

namespace cp
{
	QueueFamilyIndices FindQueueFamilies(const vk::PhysicalDevice& _device, const vk::SurfaceKHR& _surface)
	{
		QueueFamilyIndices indices;

		std::vector<vk::QueueFamilyProperties> queueFamilies = _device.getQueueFamilyProperties();

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
			{
				indices.graphicsFamily = i;
			}

			if (_device.getSurfaceSupportKHR(i, _surface))
			{
				indices.presentFamily = i;
			}

			if (indices.IsComplete())
			{
				return indices;
			}

			i++;
		}

		return indices;
	}
}