#pragma once

#include "MaterialInstance.hpp"

#include <filesystem>

namespace cp
{
	struct MaterialInstanceSaver
	{
		static bool Save(const MaterialInstance& instance, const std::filesystem::path& path);
	};
}
