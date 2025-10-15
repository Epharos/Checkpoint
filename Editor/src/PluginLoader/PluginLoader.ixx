module;

#include "../pch.hpp"
#include <iostream>
#include <filesystem>

#ifdef BUILDING_PLUGIN_LOADER
#define PLUGIN_API __declspec(dllexport)
#else
#define PLUGIN_API
#endif

export module PluginLoader;

export struct PluginContext {
	uint32_t version;

	static uint32_t MakeVersion(uint8_t major, uint8_t minor, uint16_t patch) {
		return (static_cast<uint32_t>(major) << 24) | (static_cast<uint32_t>(minor) << 16) | patch;
	}
};

export PLUGIN_API const void SayHelloFromLoader(std::string_view pluginName) {
	LOG_INFO(MF("Hello from PluginLoader to ", pluginName, "!"));
	//std::cout << "Hello from PluginLoader to " << pluginName << "!" << std::endl;
}

export class PluginLoader {
private:
	PluginContext context;
	
public:
	PluginLoader(PluginContext ctx) : context(ctx) {
		
	}

	PluginLoader() : context({0}) {
	}

	size_t ScanPlugins() {
		std::filesystem::path pluginFolder = Project::data.path.toStdString() + "/plugins";

		if (!std::filesystem::exists(pluginFolder)) {
			std::filesystem::create_directory(pluginFolder);
			LOG_WARNING("No plugins folder found, creating one.");
			return 0;
		}

		size_t pluginCount = 0;

		for (const auto& entry : std::filesystem::directory_iterator(pluginFolder)) {
			if (entry.is_directory()) {
				std::filesystem::path pluginPath = entry.path();
				std::filesystem::path manifestPath = pluginPath / "manifest.json";

				if (!std::filesystem::exists(manifestPath)) {
					LOG_WARNING(MF("No manifest.json found for plugin in folder: ", pluginPath.string()));
					continue;
				}

				std::ifstream manifestFile(manifestPath);
				if (!manifestFile.is_open()) {
					LOG_WARNING(MF("Failed to open manifest.json for plugin in folder: ", pluginPath.string()));
					continue;
				}

				nlohmann::json manifestJson;
				try {
					manifestFile >> manifestJson;
				}
				catch (const std::exception& e) {
					LOG_WARNING(MF("Failed to parse manifest.json for plugin in folder: ", pluginPath.string(), " Error: ", e.what()));
					continue;
				}

				if (!manifestJson.contains("name") || !manifestJson.contains("version") || !manifestJson.contains("entry")) {
					LOG_WARNING(MF("Invalid manifest.json for plugin in folder: ", pluginPath.string()));
					continue;
				}

				std::string pluginName = manifestJson["name"];
				std::string pluginVersion = manifestJson["version"];
				std::string entryPoint = manifestJson["entry"];

				LOG_INFO(MF("Loading plugin: ", pluginName, " Version: ", pluginVersion));
				pluginCount++;

				//TODO: Load the plugin DLL and call its initialization function with context
			}
		}

		return pluginCount;
	}
};