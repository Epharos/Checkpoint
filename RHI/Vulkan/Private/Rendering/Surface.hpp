#pragma once

#include "../pch.hpp"

namespace cp
{
	// Forward declarations
	class Instance;
	class ILogger;

	struct SurfaceInfo
	{
		void* nativeWindowHandle = nullptr;
	};

	class Surface final
	{
	public:
		Surface(ILogger & _logger, const SurfaceInfo& _info, Instance& _instance);
		~Surface();

		vk::SurfaceKHR& GetHandle() { return surface; }
		const vk::SurfaceKHR& GetHandle() const { return surface; }

	private:
		void Initialize();
		void Cleanup() const;

	private:
		vk::SurfaceKHR surface{ VK_NULL_HANDLE };

		SurfaceInfo info;
		Instance& instance;

		ILogger& logger;
	};
}