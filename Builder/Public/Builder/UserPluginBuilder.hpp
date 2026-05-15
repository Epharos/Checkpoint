#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include <Common/Plugin/UserPluginDescriptor.hpp>

namespace cp::builder
{
	struct BuildConfig
	{
		// Path to the SDK directory exported by the engine build next to the Editor executable.
		// Expected layout (populated automatically by the engine CMake post-build step):
		//   <sdkDir>/<LibName>/Public/   ← public headers, added to the plugin include path
		//   <sdkDir>/<LibName>/Private/  ← private headers, resolves relative #includes
		//   <sdkDir>/lib/<LibName>.lib   ← import lib (canonical name, no prefix/postfix)
		//
		// The per-library sub-tree mirrors the engine source tree so that ALL relative
		// #include paths (public→private and private→public) resolve correctly without
		// any source modifications.
		//
		// At runtime, resolve as: executableDir / "sdk"
		// This keeps all paths relative to the editor installation so it stays valid
		// after the editor folder is moved or shared.
		std::filesystem::path sdkDir;

		std::filesystem::path cmakeExecutable = "cmake";
		std::string buildType = "Release";

		std::filesystem::path cxxCompiler;
		std::filesystem::path cCompiler;

		std::vector<std::filesystem::path> pluginSearchPaths;
	};

	struct BuildResult
	{
		bool success = false;
		std::string output;
	};

	using BuildOutputCallback = std::function<void(std::string_view line)>;

	class UserPluginBuilder
	{
	public:
		explicit UserPluginBuilder(BuildConfig config);

		bool CreateNewPlugin(const std::filesystem::path& userPluginsDir, const std::string& name) const;

		static std::optional<UserPluginDescriptor> ParseDescriptor(const std::filesystem::path& pluginDir);
		static bool SaveDescriptor(const UserPluginDescriptor& desc, const std::filesystem::path& pluginDir);

		/**
		 * @brief Full pipeline: generate CMakeLists.txt then cmake configure then cmake build
		 */
		BuildResult Build(
			const UserPluginDescriptor& desc,
			const std::filesystem::path& pluginDir,
			BuildOutputCallback onOutput = nullptr
		) const;

		static std::filesystem::path GetOutputDllPath(const std::filesystem::path& pluginDir, const std::string& name);

	private:
		void GenerateCMakeLists(
			const UserPluginDescriptor& desc,
			const std::filesystem::path& pluginDir
		) const;

		bool RunProcess(
			const std::string& commandLine,
			const std::filesystem::path& workingDir,
			std::string& outLog,
			BuildOutputCallback onOutput
		) const;

		static std::optional<std::filesystem::path> FindLib(
			const std::filesystem::path& searchRoot,
			const std::string& libName
		);

		BuildConfig config;
	};
}
