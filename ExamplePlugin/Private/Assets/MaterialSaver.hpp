#pragma once

#include "Material.hpp"

#include <filesystem>

namespace cp
{
	struct MaterialSaver
	{
		static bool Save(const Material& material, const std::filesystem::path& path);
	};
}
