#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <Common/Plugin/UserPluginDescriptor.hpp>
#include <Common/Plugin/PluginDependencyGraph.hpp>
#include <Builder/UserPluginBuilder.hpp>

namespace cp
{
	class PluginHost;
	class ILogger;

}

namespace cp::runtime
{
	class Scene;
}

namespace cp::editor
{
	enum class UserPluginStatus
	{
		Idle,
		Building,
		Ready,
		Error
	};

	struct UserPluginInfo
	{
		UserPluginDescriptor descriptor;
		std::filesystem::path pluginDir;
		std::filesystem::path loadedDllPath;
		UserPluginStatus status = UserPluginStatus::Idle;
		std::string lastBuildOutput;
	};

	using HotReloadCallback = std::function<void(const std::string& pluginName, bool success, const std::string& buildOutput)>;
	using BuildLineCallback = std::function<void(const std::string& pluginName, std::string_view line)>;

	class UserPluginManager
	{
	public:
		UserPluginManager(
			cp::PluginHost& _pluginHost,
			cp::ILogger& _logger,
			cp::runtime::Scene& _scene
		);
		~UserPluginManager();

		void Initialize(
			const std::filesystem::path& userPluginsDir,
			const cp::builder::BuildConfig& _buildConfig
		);

		void ScanAndLoad();

		void HotReload(
			const std::string& pluginName,
			HotReloadCallback onComplete = nullptr,
			BuildLineCallback onBuildLine = nullptr
		);

		bool CreatePlugin(const std::string& name) const;

		void Tick();

		[[nodiscard]] const std::vector<UserPluginInfo>& GetPlugins() const { return plugins; }
		[[nodiscard]] const cp::PluginDependencyGraph& GetDependencyGraph() const { return dependencyGraph; }
		[[nodiscard]] bool IsBusy() const;

		[[nodiscard]] bool ConsumeStateDirty() { return std::exchange(stateDirty, false); }

		void OpenPluginFolder(const std::string& pluginName) const;

	private:
		bool LoadPlugin(UserPluginInfo& info);
		void UnloadPlugin(const std::string& name);
		void RebuildDependencyGraph();

		std::filesystem::path MakeVersionedDllCopy(
			const std::filesystem::path& srcDll,
			const std::filesystem::path& pluginDir,
			const std::string& name
		) const;

		void DoHotReload(
			const std::vector<std::string>& unloadOrder,
			const std::vector<std::string>& reloadOrder,
			HotReloadCallback onComplete,
			BuildLineCallback onBuildLine
		);

		cp::PluginHost& pluginHost;
		cp::ILogger& logger;
		cp::runtime::Scene& scene;

		std::filesystem::path userPluginsDir;
		std::unique_ptr<cp::builder::UserPluginBuilder> builder;

		std::vector<UserPluginInfo> plugins;
		cp::PluginDependencyGraph dependencyGraph;
		bool stateDirty = false;

		std::thread buildThread;
		std::mutex completionMutex;

		struct PendingCompletion
		{
			HotReloadCallback callback;
			std::string pluginName;
			bool success;
			std::string output;
		};

		std::vector<PendingCompletion> pendingCompletions;
	};
}
