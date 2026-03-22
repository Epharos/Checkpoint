#pragma once

#include <memory>
#include <filesystem>

namespace cp
{
	class RenderingHardwareInterface;

	template<typename TResource>
	struct AssetLoader
	{
		static std::shared_ptr<TResource> Load(
			const std::filesystem::path& _path,
			RenderingHardwareInterface& _rhi);
	};
}
