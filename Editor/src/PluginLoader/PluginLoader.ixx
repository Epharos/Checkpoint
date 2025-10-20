module;

#define PLUGIN_FOLDER "Plugins"

#include "../pch.hpp"
#include <iostream>
#include <filesystem>
#include <format>
#include <windows.h> // For Windows only, TODO: change it so it works on other platforms too
#include "../macros.hpp"

export module PluginLoader;
export import PluginContext;


export namespace cp {
	class PluginLoader {
	private:
		PluginContext context;

	public:
		PluginLoader(PluginContext ctx) : context(ctx) {

		}

		size_t ScanPlugins() {
			std::filesystem::path pluginFolder = std::format("{}/{}", Project::data.path.toStdString(), PLUGIN_FOLDER);

			if (!std::filesystem::exists(pluginFolder)) {
				std::filesystem::create_directory(pluginFolder);
				LOG_WARNING("No plugins folder found, creating one.");
				return 0;
			}

			size_t pluginCount = 0;

			typedef void(__stdcall* PluginEntryPoint)(const cp::PluginContext&);

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
					std::string pluginEntryPoint = manifestJson["entry"];

					LOG_INFO(MF("Loading plugin: ", pluginName, " (Version: ", pluginVersion, ")"));
					pluginCount++;

					std::filesystem::path dllFile = pluginPath / (pluginName + ".dll");

					HINSTANCE hGetProcIDDLL = LoadLibrary(dllFile.c_str());

					if (!hGetProcIDDLL) {
						std::cerr << "Could not load the dynamic library: " << dllFile << std::endl;
						continue;
					}

					PluginEntryPoint entryPoint = (PluginEntryPoint)GetProcAddress(hGetProcIDDLL, pluginEntryPoint.c_str());

					if (!entryPoint) {
						std::cerr << "Could not locate the function: " << pluginEntryPoint << " in " << dllFile << std::endl;
						FreeLibrary(hGetProcIDDLL);
						continue;
					}

					std::cout << std::endl;

					entryPoint(context);
				}
			}

			return pluginCount;
		}
	};
}