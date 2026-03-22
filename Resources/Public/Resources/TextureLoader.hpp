#pragma once

#include "AssetLoader.hpp"
#include <RHI/Data.hpp>

#include <string>
#include <memory>

namespace cp
{
	class RenderingHardwareInterface;

	template<>
	struct AssetLoader<ITexture>
	{
		static std::shared_ptr<ITexture> Load(
			const std::filesystem::path& path,
			RenderingHardwareInterface& rhi);
	};
}
