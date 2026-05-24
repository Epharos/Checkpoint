#pragma once

#include "Material.hpp"

#include <Common/Plugin/AssetAuthoring.hpp>
#include <ShaderCompiler/ShaderReflection.hpp>

#include <filesystem>
#include <unordered_map>

namespace cp
{
	class ShaderCompiler;
	class AssetRegistry;

	class MaterialAuthoring final : public IAssetAuthoring
	{
	public:
		MaterialAuthoring(ShaderCompiler& _compiler, AssetRegistry& _assetRegistry);

		[[nodiscard]] std::string_view FileExtension() const override;

		[[nodiscard]] std::vector<AuthoringSectionDescriptor> BuildSections(
			const std::filesystem::path& _assetPath
		) const override;

		bool ApplyValue(
			const std::filesystem::path& _assetPath,
			std::string_view _sectionId,
			std::string_view _fieldId,
			const AuthoringValue& _value,
			std::string& _outError
		) const override;

	private:
		[[nodiscard]] const ShaderReflection* GetOrReflect(
			const std::filesystem::path& _shaderPath
		) const;

		ShaderCompiler& compiler;
		AssetRegistry& assetRegistry;
		mutable std::unordered_map<std::string, ShaderReflection> reflectionCache;
	};
}
