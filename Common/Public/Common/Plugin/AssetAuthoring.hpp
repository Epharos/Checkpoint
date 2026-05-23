#pragma once

#include <Common/Authoring.hpp>

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace cp
{
	class IAssetAuthoring
	{
	public:
		virtual ~IAssetAuthoring() = default;

		// File extension without leading dot, e.g. "matinst"
		[[nodiscard]] virtual std::string_view FileExtension() const = 0;

		[[nodiscard]] virtual std::vector<AuthoringSectionDescriptor> BuildSections(
			const std::filesystem::path& assetPath
		) const = 0;

		virtual bool ApplyValue(
			const std::filesystem::path& assetPath,
			std::string_view sectionId,
			std::string_view fieldId,
			const AuthoringValue& value,
			std::string& outError
		) const = 0;
	};
}
