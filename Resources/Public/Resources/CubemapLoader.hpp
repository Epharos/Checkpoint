#pragma once

#include <filesystem>
#include <memory>

namespace cp
{
	class ITexture;
	class RenderingHardwareInterface;

	/**
	 * @brief Loads a cubemap texture from 6 face images.
	 *
	 * Face images are resolved by appending a suffix to basePath:
	 *   {basePath}_px.png  (+X / right)
	 *   {basePath}_nx.png  (-X / left)
	 *   {basePath}_py.png  (+Y / up)
	 *   {basePath}_ny.png  (-Y / down)
	 *   {basePath}_pz.png  (+Z / front)
	 *   {basePath}_nz.png  (-Z / back)
	 *
	 * All faces must have identical dimensions and channel count.
	 * Returns nullptr on any loading failure.
	 */
	std::shared_ptr<ITexture> LoadCubemapTexture(
		const std::filesystem::path& basePath,
		RenderingHardwareInterface& rhi
	);
}
