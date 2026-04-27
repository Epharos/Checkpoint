#pragma once

#include "AssetLoader.hpp"
#include "Mesh.hpp"

namespace cp
{
	class RenderingHardwareInterface;

	template<>
	struct AssetLoader<Mesh>
	{
		static std::shared_ptr<Mesh> Load(
			const std::filesystem::path& _path,
			RenderingHardwareInterface& _rhi);
	};
}
