#pragma once

#include "MaterialInstance.hpp"
#include <Common/Plugin/AssetAuthoring.hpp>
#include <ShaderCompiler/ShaderReflection.hpp>

#include <filesystem>
#include <string>
#include <unordered_map>

namespace cp
{
	class ShaderCompiler;
	class AssetRegistry;

	class MaterialInstanceAuthoring final : public IAssetAuthoring
	{
	public:
		MaterialInstanceAuthoring(ShaderCompiler& compiler, AssetRegistry& assetRegistry);

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
		[[nodiscard]] const ShaderReflection* GetOrCompileReflection(
			const std::filesystem::path& _shaderPath
		) const;

		ShaderCompiler& compiler;
		AssetRegistry& assetRegistry;
		mutable std::unordered_map<std::string, ShaderReflection> reflectionCache;
	};
}
