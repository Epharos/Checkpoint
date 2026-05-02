#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "PluginAPI.hpp"

namespace cp
{
	class PluginHost final
	{
	public:
		explicit PluginHost(PluginHostContext _context = {});
		~PluginHost();

		PluginHost(const PluginHost&) = delete;
		PluginHost& operator=(const PluginHost&) = delete;

		PluginHost(PluginHost&& _other) noexcept;
		PluginHost& operator=(PluginHost&& _other) noexcept;

		bool LoadPlugin(const std::filesystem::path& _path);
		size_t LoadPluginsFromDirectory(const std::filesystem::path& _directory, bool _recursive = false);

		bool UnloadPlugin(std::string_view _pluginName);
		void UnloadAll();

		[[nodiscard]] bool IsPluginLoaded(std::string_view _pluginName) const;
		[[nodiscard]] std::vector<std::string> GetLoadedPluginNames() const;
		[[nodiscard]] const std::string& GetLastError() const;

	private:
		struct LoadedPlugin
		{
			std::string name;
			std::filesystem::path path;
			void* nativeHandle = nullptr;
			PluginShutdownRuntimeFn shutdownRuntime = nullptr;
			PluginShutdownEditorFn shutdownEditor = nullptr;
			bool runtimeRegistered = false;
			bool editorRegistered = false;
		};

		PluginHostContext context;
		std::vector<LoadedPlugin> plugins;
		std::string lastError;
	};
}
