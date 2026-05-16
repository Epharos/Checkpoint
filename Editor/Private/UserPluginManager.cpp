#include "UserPluginManager.hpp"

#include <chrono>
#include <format>
#include <unordered_set>

#include <Common/Core/Log.hpp>
#include <Common/Plugin/PluginHost.hpp>
#include <Runtime/Scene/Scene.hpp>
#include <Runtime/Scene/SceneSerializer.hpp>

#if defined(CP_PLATFORM_WINDOWS)
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <Windows.h>
	#include <shellapi.h>
#endif

namespace cp::editor
{
	namespace
	{
		std::string TimestampSuffix()
		{
			const auto now = std::chrono::system_clock::now();
			const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
				now.time_since_epoch()).count();
			return std::to_string(ms);
		}
	}

	UserPluginManager::UserPluginManager(cp::PluginHost& _pluginHost, cp::ILogger& _logger, cp::runtime::Scene& _scene)
	: pluginHost(_pluginHost), logger(_logger), scene(_scene) {}

	UserPluginManager::~UserPluginManager()
	{
		if (buildThread.joinable())
		{
			buildThread.join();
		}
	}

	void UserPluginManager::Initialize(
		const std::filesystem::path& _dir,
		const cp::builder::BuildConfig& _buildConfig
	)
	{
		userPluginsDir = _dir;
		builder = std::make_unique<cp::builder::UserPluginBuilder>(_buildConfig);
		std::filesystem::create_directories(userPluginsDir);
	}

	void UserPluginManager::ScanAndLoad()
	{
		if (!std::filesystem::exists(userPluginsDir))
		{
			return;
		}

		stateDirty = true;

		std::error_code ec;
		for (const auto& entry : std::filesystem::directory_iterator(userPluginsDir, ec))
		{
			if (!entry.is_directory())
			{
				continue;
			}

			const std::filesystem::path pluginDir = entry.path();
			const std::filesystem::path jsonPath = pluginDir / "plugin.json";

			if (!std::filesystem::exists(jsonPath))
			{
				continue;
			}

			auto descOpt = cp::builder::UserPluginBuilder::ParseDescriptor(pluginDir);
			if (!descOpt)
			{
				logger.Log(CP_LOG_EVENT(
					cp::ILogger::Warning,
					"UserPlugins",
					cp::Message::Create("Could not parse plugin.json in {}", pluginDir.string())
				));

				continue;
			}

			UserPluginInfo info;
			info.descriptor = std::move(*descOpt);
			info.pluginDir = pluginDir;
			plugins.push_back(std::move(info));
		}

		RebuildDependencyGraph();

		std::string graphError;
		const std::vector<std::string> loadOrder = dependencyGraph.GetLoadOrder(&graphError);
		if (!graphError.empty())
		{
			logger.Log(CP_LOG_EVENT(
				cp::ILogger::Error,
				"UserPlugins",
				cp::Message::Create("Plugin dependency cycle: {}", graphError)
			));

			return;
		}

		for (const auto& name : loadOrder)
		{
			const auto it = std::ranges::find_if(plugins,
				[&](const UserPluginInfo& p) { return p.descriptor.name == name; });

			if (it == plugins.end())
			{
				continue;
			}

			const std::filesystem::path dllPath = cp::builder::UserPluginBuilder::GetOutputDllPath(it->pluginDir, name);
			if (!std::filesystem::exists(dllPath))
			{
				logger.Log(CP_LOG_EVENT(
					cp::ILogger::Info,
					"UserPlugins",
					cp::Message::Create("No compiled DLL for '{}' — recompile to load it.", name)
				));

				it->status = UserPluginStatus::Idle;
				continue;
			}

			LoadPlugin(*it);
		}
	}

	bool UserPluginManager::LoadPlugin(UserPluginInfo& info)
	{
		const std::filesystem::path srcDll =
			cp::builder::UserPluginBuilder::GetOutputDllPath(info.pluginDir, info.descriptor.name);

		const std::filesystem::path versionedDll = MakeVersionedDllCopy(srcDll, info.pluginDir, info.descriptor.name);

		if (versionedDll.empty())
		{
			logger.Log(CP_LOG_EVENT(
				cp::ILogger::Error,
				"UserPlugins",
				cp::Message::Create(
					"DLL not found for '{}' — expected: {}",
					info.descriptor.name,
					srcDll.string()
				)
			));

			info.status = UserPluginStatus::Error;
			stateDirty  = true;
			return false;
		}

		if (!pluginHost.LoadPlugin(versionedDll, info.pluginDir))
		{
			logger.Log(CP_LOG_EVENT(
				cp::ILogger::Error,
				"UserPlugins",
				cp::Message::Create("Failed to load '{}': {}", info.descriptor.name, pluginHost.GetLastError())
			));

			info.status = UserPluginStatus::Error;
			stateDirty  = true;
			return false;
		}

		info.loadedDllPath = versionedDll;
		info.status = UserPluginStatus::Ready;
		stateDirty = true;

		logger.Log(CP_LOG_EVENT(
			cp::ILogger::Info,
			"UserPlugins",
			cp::Message::Create("Loaded user plugin '{}'", info.descriptor.name)
		));

		return true;
	}

	void UserPluginManager::UnloadPlugin(const std::string& name)
	{
		if (!pluginHost.IsPluginLoaded(name))
		{
			return;
		}

		pluginHost.UnloadPlugin(name);

		const auto it = std::ranges::find_if(plugins,
			[&](const UserPluginInfo& p) { return p.descriptor.name == name; });

		if (it != plugins.end())
		{
			it->status = UserPluginStatus::Idle;
		}

		stateDirty = true;
		logger.Log(CP_LOG_EVENT(
			cp::ILogger::Info,
			"UserPlugins",
			cp::Message::Create("Unloaded user plugin '{}'", name)
		));
	}

	void UserPluginManager::RebuildDependencyGraph()
	{
		dependencyGraph.Clear();

		for (const auto& info : plugins)
		{
			dependencyGraph.AddPlugin(info.descriptor.name, info.descriptor.pluginDeps);
		}
	}

	std::filesystem::path UserPluginManager::MakeVersionedDllCopy(
		const std::filesystem::path& srcDll,
		const std::filesystem::path& pluginDir,
		const std::string& name
	) const
	{
		if (!std::filesystem::exists(srcDll))
		{
			return {};
		}

		const std::filesystem::path destDir = pluginDir / "_build" / "loaded";
		std::filesystem::create_directories(destDir);

		{
			std::vector<std::filesystem::path> old;
			std::error_code ec;
			for (const auto& e : std::filesystem::directory_iterator(destDir, ec))
			{
				if (e.is_regular_file() && e.path().extension() == ".dll")
				{
					old.push_back(e.path());
				}
			}

			std::ranges::sort(old);

			while (old.size() >= 3)
			{
				std::filesystem::remove(old.front(), ec);
				old.erase(old.begin());
			}
		}

		const std::filesystem::path dest = destDir / (name + "_" + TimestampSuffix() + ".dll");
		std::error_code ec;
		std::filesystem::copy_file(srcDll, dest, std::filesystem::copy_options::overwrite_existing, ec);
		if (ec)
		{
			return {};
		}

		return dest;
	}

	void UserPluginManager::HotReload(
		const std::string& pluginName,
		HotReloadCallback onComplete,
		BuildLineCallback onBuildLine
	)
	{
		if (IsBusy())
		{
			logger.Log(CP_LOG_EVENT(
				cp::ILogger::Warning,
				"UserPlugins",
				cp::Message::Create("Hot-reload already in progress, ignoring request for '{}'", pluginName)
			));

			return;
		}

		std::string graphError;
		const std::vector<std::string> loadOrder = dependencyGraph.GetLoadOrder(&graphError);
		const std::vector<std::string> unloadOrder = dependencyGraph.GetUnloadOrder(pluginName);

		if (!graphError.empty())
		{
			logger.Log(CP_LOG_EVENT(
				cp::ILogger::Error,
				"UserPlugins",
				cp::Message::Create("Cannot hot-reload '{}': {}", pluginName, graphError)
			));

			if (onComplete)
			{
				onComplete(pluginName, false, graphError);
			}

			return;
		}

		const std::vector<std::string> affected = dependencyGraph.GetAffectedSet(pluginName);
		const std::unordered_set<std::string> affectedSet(affected.begin(), affected.end());

		std::vector<std::string> reloadOrder;
		for (const auto& n : loadOrder)
		{
			if (affectedSet.contains(n))
			{
				reloadOrder.push_back(n);
			}
		}

		buildThread = std::thread([this, unloadOrder, reloadOrder, onComplete, onBuildLine]()
		{
			DoHotReload(unloadOrder, reloadOrder, onComplete, onBuildLine);
		});

		buildThread.detach();
	}

	void UserPluginManager::DoHotReload(
		const std::vector<std::string>& unloadOrder,
		const std::vector<std::string>& reloadOrder,
		HotReloadCallback onComplete,
		BuildLineCallback onBuildLine
	)
	{
		std::string combinedOutput;
		bool allSucceeded = false;

		try
		{
			const std::filesystem::path tempScene = userPluginsDir / ".hotreload_scene_backup.tmp";
			if (!cp::runtime::SceneSerializer::SaveSceneToFile(scene, tempScene, &logger))
			{
				logger.Log(CP_LOG_EVENT(
					ILogger::Error,
					"UserPlugins",
					cp::Message::Create("Cannot save scene backup '{}'", tempScene.string())
				));
			};

			scene.ShutdownSystems();

			for (const auto& name : unloadOrder)
			{
				UnloadPlugin(name);
			}

			allSucceeded = true;

			for (const auto& name : reloadOrder)
			{
				const auto it = std::ranges::find_if(plugins,
					[&](const UserPluginInfo& p) { return p.descriptor.name == name; });

				if (it == plugins.end()) continue;

				it->status = UserPluginStatus::Building;
				stateDirty = true;

				if (auto freshDesc = cp::builder::UserPluginBuilder::ParseDescriptor(it->pluginDir))
				{
					it->descriptor = std::move(*freshDesc);
					RebuildDependencyGraph();
				}

				const auto buildLineCb = onBuildLine
					? builder::BuildOutputCallback([&](std::string_view line) { onBuildLine(name, line); })
					: builder::BuildOutputCallback{};

				const cp::builder::BuildResult result = builder->Build(it->descriptor, it->pluginDir, buildLineCb);

				it->lastBuildOutput = result.output;
				combinedOutput += "[" + name + "]\n" + result.output + "\n";

				if (!result.success)
				{
					it->status = UserPluginStatus::Error;
					stateDirty = true;
					allSucceeded = false;

					logger.Log(CP_LOG_EVENT(
						cp::ILogger::Error,
						"UserPlugins",
						cp::Message::Create("Build failed for '{}'", name)
					));

					break;
				}
			}

			if (allSucceeded)
			{
				for (const auto& name : reloadOrder)
				{
					const auto it = std::ranges::find_if(plugins,
						[&](const UserPluginInfo& p) { return p.descriptor.name == name; });

					if (it != plugins.end())
					{
						LoadPlugin(*it);
					}
				}
			}

			if (std::filesystem::exists(tempScene))
			{
				if (!cp::runtime::SceneSerializer::LoadSceneFromFile(scene, tempScene, &logger))
				{
					logger.Log(CP_LOG_EVENT(
						ILogger::Error,
						"UserPlugins",
						cp::Message::Create("Cannot load scene backup '{}'", tempScene.string())
					));
				}

				std::filesystem::remove(tempScene);
			}
		}
		catch (const std::exception& e)
		{
			const std::string msg = std::string("Hot-reload exception: ") + e.what();
			combinedOutput += "\n" + msg + "\n";
			allSucceeded = false;

			logger.Log(CP_LOG_EVENT(
				cp::ILogger::Error,
				"UserPlugins",
				cp::Message::Create("{}", msg)
			));

			for (const auto& name : reloadOrder)
			{
				const auto it = std::ranges::find_if(plugins,
					[&](const UserPluginInfo& p) { return p.descriptor.name == name; });

				if (it != plugins.end() && it->status == UserPluginStatus::Building)
				{
					it->status = UserPluginStatus::Error;
					stateDirty = true;
				}
			}
		}

		{
			std::lock_guard lock(completionMutex);
			pendingCompletions.push_back({
				onComplete,
				reloadOrder.empty() ? std::string{} : reloadOrder.front(),
				allSucceeded,
				combinedOutput
			});
		}
	}

	void UserPluginManager::Tick()
	{
		std::vector<PendingCompletion> toDispatch;
		{
			std::lock_guard lock(completionMutex);
			toDispatch.swap(pendingCompletions);
		}

		if (toDispatch.empty())
		{
			return;
		}

		stateDirty = true;

		for (auto& completion : toDispatch)
		{
			if (completion.callback)
			{
				completion.callback(completion.pluginName, completion.success, completion.output);
			}
		}
	}

	bool UserPluginManager::IsBusy() const
	{
		return std::ranges::any_of(plugins,
			[](const UserPluginInfo& p) { return p.status == UserPluginStatus::Building; });
	}

	bool UserPluginManager::CreatePlugin(const std::string& name) const
	{
		if (!builder)
		{
			return false;
		}

		return builder->CreateNewPlugin(userPluginsDir, name);
	}

	void UserPluginManager::OpenPluginFolder(const std::string& pluginName) const
	{
		const auto it = std::ranges::find_if(plugins,
			[&](const UserPluginInfo& p) { return p.descriptor.name == pluginName; });

		if (it == plugins.end())
		{
			return;
		}

#if defined(CP_PLATFORM_WINDOWS)
		const std::string path = it->pluginDir.string();
		ShellExecuteA(nullptr, "explore", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#else
	#error Undefined behaviour for this platform
#endif
	}
}
