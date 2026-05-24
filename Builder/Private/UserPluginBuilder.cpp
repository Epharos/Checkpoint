#include "../Public/Builder/UserPluginBuilder.hpp"

#include <Common/Core/Macros.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>

#include <nlohmann/json.hpp>

#if defined(CP_PLATFORM_WINDOWS)
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <Windows.h>
#endif

namespace cp::builder
{
	namespace
	{
		void Replace(std::string& str, std::string_view placeholder, std::string_view replacement)
		{
			std::size_t pos = 0;
			while ((pos = str.find(placeholder, pos)) != std::string::npos)
			{
				str.replace(pos, placeholder.size(), replacement);
				pos += replacement.size();
			}
		}

		std::string ForwardSlashes(std::filesystem::path path)
		{
			std::string s = path.string();
			std::ranges::replace(s, '\\', '/');
			return s;
		}

		constexpr std::string_view EntryPointTemplate = R"(
#include <Common/Plugin/PluginAPI.hpp>

extern bool Register@NAME@Runtime(cp::PluginRuntimeContext& _context);
extern void Shutdown@NAME@Runtime(cp::PluginRuntimeContext& _context);
extern bool Register@NAME@Editor(cp::PluginEditorContext& _context);
extern void Shutdown@NAME@Editor(cp::PluginEditorContext& _context);

CP_DECLARE_PLUGIN(
    "@NAME@",
    Register@NAME@Runtime,
    Shutdown@NAME@Runtime,
    Register@NAME@Editor,
    Shutdown@NAME@Editor
)
)";

		constexpr std::string_view RuntimeTemplate = R"(
#include <Common/Plugin/PluginAPI.hpp>
#include <Common/Plugin/PluginRegistryNames.hpp>

bool Register@NAME@Runtime(cp::PluginRuntimeContext& _context)
{
    if (_context.registryManager == nullptr)
        return false;

    // Register your components, systems and renderpasses here.
    // Example:
    //   auto& compReg = _context.registryManager->GetOrCreate<cp::ecs::IComponentRegistrar>(
    //       std::string(cp::EcsComponentRegistryName));
    //   compReg.RegisterType<MyComponentRegistrar>("MyComponent");

    return true;
}

void Shutdown@NAME@Runtime(cp::PluginRuntimeContext& _context)
{
    // Unregister your types here (mirror what you registered above).
}
)";

		constexpr std::string_view EditorTemplate = R"(
#include <Common/Plugin/PluginAPI.hpp>
#include <Common/Plugin/PluginRegistryNames.hpp>

bool Register@NAME@Editor(cp::PluginEditorContext& _context)
{
    if (_context.registryManager == nullptr)
        return false;

    // Register editor authoring types here.

    return true;
}

void Shutdown@NAME@Editor(cp::PluginEditorContext& _context)
{
    // Unregister editor types here.
}
)";

		std::string ApplyNameTemplate(std::string_view tmpl, const std::string& name)
		{
			std::string result(tmpl);
			Replace(result, "@NAME@", name);
			return result;
		}

		// ------------------------------------------------------------------ CMakeLists template
		// Placeholders filled in by GenerateCMakeLists:
		//   @NAME@ : Plugin name
		//   @PLUGIN_DIR@ : Absolute path to plugin root (forward slashes)
		//   @EXTRA_INCLUDES@ : Newline-separated quoted include dirs
		//   @ENGINE_LIBS@ : Newline-separated quoted absolute .lib paths (sdk/lib/)
		//   @LOCAL_LIBS@ : Newline-separated quoted absolute .lib paths
		//   @PLUGIN_DEP_LIBS@ : Newline-separated quoted absolute .lib paths
		//   @GITHUB_DEPS@ : FetchContent blocks (may be empty)
		//   @GITHUB_LINK_LIBS@ : Target names from github deps

		constexpr std::string_view CMakeTemplate = R"(
cmake_minimum_required(VERSION 3.20)
project(@NAME@)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

@SDK_INCLUDE@

@GITHUB_DEPS@

file(GLOB_RECURSE SOURCES
    "@PLUGIN_DIR@/Sources/*.cpp"
    "@PLUGIN_DIR@/Sources/*.hpp"
    "@PLUGIN_DIR@/Public/*.hpp"
)

set(CMAKE_SHARED_LIBRARY_PREFIX "")

add_library(@NAME@ SHARED ${SOURCES})

target_include_directories(@NAME@
    PUBLIC
        "@PLUGIN_DIR@/Public"
    PRIVATE
        "@PLUGIN_DIR@/Sources"
@EXTRA_INCLUDES@
)

target_link_libraries(@NAME@
    PRIVATE
@ENGINE_TARGETS@
@LOCAL_LIBS@
@PLUGIN_DEP_LIBS@
@GITHUB_LINK_LIBS@
)

set_target_properties(@NAME@ PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "@PLUGIN_DIR@/_build/out"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "@PLUGIN_DIR@/_build/out"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "@PLUGIN_DIR@/_build/out"
    ARCHIVE_OUTPUT_DIRECTORY "@PLUGIN_DIR@/_build/out"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "@PLUGIN_DIR@/_build/out"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "@PLUGIN_DIR@/_build/out"
)
)";
	}

	UserPluginBuilder::UserPluginBuilder(BuildConfig config)
		: config(std::move(config))	{}

	bool UserPluginBuilder::CreateNewPlugin(
		const std::filesystem::path& userPluginsDir,
		const std::string& name
	) const
	{
		const std::filesystem::path pluginDir = userPluginsDir / name;
		if (std::filesystem::exists(pluginDir))
		{
			return false;
		}

		std::filesystem::create_directories(pluginDir / "Sources");
		std::filesystem::create_directories(pluginDir / "Public");
		std::filesystem::create_directories(pluginDir / "_build");

		const UserPluginDescriptor desc
		{
			.name = name,
			.engineLibs = {
				"Common",
				"ECS"
			}
		};

		if (!SaveDescriptor(desc, pluginDir))
		{
			return false;
		}

		auto writeFile = [](const std::filesystem::path& path, const std::string& content) -> bool
		{
			std::ofstream f(path);
			if (!f.is_open()) return false;
			f << content;
			return true;
		};

		const bool ok =
			writeFile(pluginDir / "Sources" / (name + ".cpp"), ApplyNameTemplate(EntryPointTemplate, name))
			&& writeFile(pluginDir / "Sources" / (name + "Runtime.cpp"), ApplyNameTemplate(RuntimeTemplate, name))
			&& writeFile(pluginDir / "Sources" / (name + "Editor.cpp"), ApplyNameTemplate(EditorTemplate, name));

		return ok;
	}

	std::optional<UserPluginDescriptor> UserPluginBuilder::ParseDescriptor(const std::filesystem::path& pluginDir)
	{
		const std::filesystem::path jsonPath = pluginDir / "plugin.json";
		if (!std::filesystem::exists(jsonPath))
		{
			return std::nullopt;
		}

		std::ifstream f(jsonPath);
		if (!f.is_open())
		{
			return std::nullopt;
		}

		try
		{
			const nlohmann::json j = nlohmann::json::parse(f);

			UserPluginDescriptor desc;
			desc.name = j.value("name", pluginDir.stem().string());
			desc.pluginDeps = j.value("pluginDeps", std::vector<std::string>{});
			desc.engineLibs = j.value("engineLibs", std::vector<std::string>{});

			if (j.contains("localLibs"))
			{
				for (const auto& entry : j["localLibs"])
				{
					LocalLibraryDependency d;
					d.includePath = entry.value("includePath", "");
					d.libPath = entry.value("libPath", "");
					desc.localLibs.push_back(std::move(d));
				}
			}

			if (j.contains("githubDeps"))
			{
				for (const auto& entry : j["githubDeps"])
				{
					GitDependency d;
					d.repo = entry.value("repo", "");
					d.tag = entry.value("tag", "");
					d.linkTarget = entry.value("linkTarget", "");
					if (!d.repo.empty())
					{
						desc.githubDeps.push_back(std::move(d));
					}
				}
			}

			return desc;
		}
		catch (...)
		{
			return std::nullopt;
		}
	}

	bool UserPluginBuilder::SaveDescriptor(const UserPluginDescriptor& desc, const std::filesystem::path& pluginDir)
	{
		nlohmann::json j;
		j["name"] = desc.name;
		j["pluginDeps"] = desc.pluginDeps;
		j["engineLibs"] = desc.engineLibs;

		{
			nlohmann::json localLibsJson = nlohmann::json::array();
			for (const auto& lib : desc.localLibs)
			{
				nlohmann::json entry;
				entry["includePath"] = lib.includePath;
				entry["libPath"] = lib.libPath;
				localLibsJson.push_back(entry);
			}
			j["localLibs"] = localLibsJson;
		}

		{
			nlohmann::json githubJson = nlohmann::json::array();
			for (const auto& dep : desc.githubDeps)
			{
				nlohmann::json entry;
				entry["repo"] = dep.repo;
				entry["tag"] = dep.tag;

				if (!dep.linkTarget.empty())
				{
					entry["linkTarget"] = dep.linkTarget;
				}

				githubJson.push_back(entry);
			}
			j["githubDeps"] = githubJson;
		}

		std::ofstream f(pluginDir / "plugin.json");
		if (!f.is_open())
		{
			return false;
		}

		f << j.dump(4);
		return true;
	}

	// -----------------------------------------------------------------------

	std::filesystem::path UserPluginBuilder::GetOutputDllPath(
		const std::filesystem::path& pluginDir,
		const std::string& name
	)
	{
		return pluginDir / "_build" / "out" / (name + ".dll");
	}

	// -----------------------------------------------------------------------

	std::optional<std::filesystem::path> UserPluginBuilder::FindLib(
		const std::filesystem::path& searchRoot,
		const std::string& libName
	)
	{
		if (!std::filesystem::exists(searchRoot))
		{
			return std::nullopt;
		}

		const std::string filename = libName + ".lib";

		std::error_code ec;
		for (const auto& entry : std::filesystem::recursive_directory_iterator(searchRoot, ec))
		{
			if (entry.is_regular_file() && entry.path().filename().string() == filename)
			{
				return entry.path();
			}
		}

		return std::nullopt;
	}

	void UserPluginBuilder::GenerateCMakeLists(
		const UserPluginDescriptor& desc,
		const std::filesystem::path& pluginDir
	) const
	{
		std::string sdkInclude;
		std::string extraIncludes;
		std::string engineTargetLines;
		std::string localLibLines;
		std::string pluginDepLibLines;
		std::string githubDeps;
		std::string githubLinkLibs;

		if (!config.sdkDir.empty())
		{
			const std::filesystem::path sdkCmake = config.sdkDir / "EngineSDK.cmake";
			sdkInclude = "include(\"" + ForwardSlashes(sdkCmake) + "\")";

			for (const auto& libName : desc.engineLibs)
			{
				engineTargetLines += "        cp::" + libName + "\n";
			}
		}

		for (const auto& local : desc.localLibs)
		{
			if (!local.includePath.empty())
			{
				extraIncludes += "        \"" + local.includePath + "\"\n";
			}

			if (!local.libPath.empty())
			{
				localLibLines += "        \"" + local.libPath + "\"\n";
			}
		}

		for (const auto& depName : desc.pluginDeps)
		{
			for (const auto& searchPath : config.pluginSearchPaths)
			{
				const std::filesystem::path depPublic = searchPath / depName / "Public";
				if (!std::filesystem::exists(depPublic))
				{
					continue;
				}

				extraIncludes += "        \"" + ForwardSlashes(depPublic) + "\"\n";

				const std::filesystem::path depLibDir = searchPath / depName / "_build" / "out";
				if (const auto depLib = FindLib(depLibDir, depName))
				{
					pluginDepLibLines += "        \"" + ForwardSlashes(*depLib) + "\"\n";
				}

				break; // first match wins
			}
		}

		for (const auto& gh : desc.githubDeps)
		{
			const std::string repoName = gh.repo.find('/') != std::string::npos
				? gh.repo.substr(gh.repo.find('/') + 1)
				: gh.repo;

			const std::string linkTarget = gh.linkTarget.empty() ? repoName : gh.linkTarget;

			githubDeps += "include(FetchContent)\n";
			githubDeps += "FetchContent_Declare(\n";
			githubDeps += "    " + repoName + "\n";
			githubDeps += "    GIT_REPOSITORY https://github.com/" + gh.repo + ".git\n";
			githubDeps += "    GIT_TAG " + gh.tag + "\n";
			githubDeps += "    EXCLUDE_FROM_ALL\n";
			githubDeps += ")\n";
			githubDeps += "FetchContent_MakeAvailable(" + repoName + ")\n\n";

			githubLinkLibs += "        " + linkTarget + "\n";
		}

		std::string cmake(CMakeTemplate);
		Replace(cmake, "@NAME@", desc.name);
		Replace(cmake, "@PLUGIN_DIR@", ForwardSlashes(pluginDir));
		Replace(cmake, "@SDK_INCLUDE@", sdkInclude);
		Replace(cmake, "@EXTRA_INCLUDES@", extraIncludes);
		Replace(cmake, "@ENGINE_TARGETS@", engineTargetLines);
		Replace(cmake, "@LOCAL_LIBS@", localLibLines);
		Replace(cmake, "@PLUGIN_DEP_LIBS@", pluginDepLibLines);
		Replace(cmake, "@GITHUB_DEPS@", githubDeps);
		Replace(cmake, "@GITHUB_LINK_LIBS@", githubLinkLibs);

		std::filesystem::create_directories(pluginDir / "_build");
		std::ofstream f(pluginDir / "_build" / "CMakeLists.txt");
		f << cmake;
	}

	bool UserPluginBuilder::RunProcess(
		const std::string& commandLine,
		const std::filesystem::path& workingDir,
		std::string& outLog,
		BuildOutputCallback onOutput
	) const
	{
#if defined(CP_PLATFORM_WINDOWS)
		// Redirect stdout + stderr through a pipe so we can stream each line.
		HANDLE readPipe = nullptr;
		HANDLE writePipe = nullptr;
		SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
		if (!CreatePipe(&readPipe, &writePipe, &sa, 0))
			return false;
		SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);

		STARTUPINFOA si{};
		si.cb = sizeof(STARTUPINFOA);
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdOutput = writePipe;
		si.hStdError = writePipe;
		si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

		PROCESS_INFORMATION pi{};
		std::string cmd = commandLine;
		const std::string workDir = workingDir.string();

		const BOOL created = CreateProcessA(
			nullptr,
			cmd.data(),
			nullptr, nullptr,
			TRUE,
			CREATE_NO_WINDOW,
			nullptr,
			workDir.c_str(),
			&si,
			&pi
		);

		CloseHandle(writePipe);

		if (!created)
		{
			CloseHandle(readPipe);
			return false;
		}

		char buffer[4096];
		DWORD bytesRead = 0;
		std::string lineAccum;
		while (ReadFile(readPipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0)
		{
			buffer[bytesRead] = '\0';
			outLog += buffer;
			if (onOutput)
			{
				// Deliver complete lines to the callback
				lineAccum += buffer;
				std::size_t pos = 0;
				std::size_t newline = 0;

				while ((newline = lineAccum.find('\n', pos)) != std::string::npos)
				{
					onOutput(std::string_view(lineAccum).substr(pos, newline - pos + 1));
					pos = newline + 1;
				}

				lineAccum = lineAccum.substr(pos);
			}
		}

		if (onOutput && !lineAccum.empty())
		{
			onOutput(lineAccum);
		}

		CloseHandle(readPipe);
		WaitForSingleObject(pi.hProcess, INFINITE);

		DWORD exitCode = 1;
		GetExitCodeProcess(pi.hProcess, &exitCode);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		return exitCode == 0;
#else
		// Fallback: not implemented for non-Windows yet
		outLog = "UserPluginBuilder: process execution not implemented on this platform.";
		#warning Process execution not implemented on this platform
		return false;
#endif
	}

	BuildResult UserPluginBuilder::Build(
		const UserPluginDescriptor& desc,
		const std::filesystem::path& pluginDir,
		BuildOutputCallback onOutput
	) const
	{
		BuildResult result;

		GenerateCMakeLists(desc, pluginDir);

		const std::filesystem::path buildDir = pluginDir / "_build" / "cmake_out";
		std::filesystem::create_directories(buildDir);

		const std::string cmakeExe = "\"" + config.cmakeExecutable.string() + "\"";
		const std::string cmakeSrc = "\"" + ForwardSlashes(pluginDir / "_build") + "\"";
		const std::string cmakeOut = "\"" + ForwardSlashes(buildDir) + "\"";

		std::string configureCmd =
			cmakeExe + " -S " + cmakeSrc + " -B " + cmakeOut +
			" -G Ninja" +
			" -DCMAKE_BUILD_TYPE=" + config.buildType;

		if (!config.cxxCompiler.empty())
		{
			configureCmd += " -DCMAKE_CXX_COMPILER=\"" + ForwardSlashes(config.cxxCompiler) + "\"";
		}

		if (!config.cCompiler.empty())
		{
			configureCmd += " -DCMAKE_C_COMPILER=\"" + ForwardSlashes(config.cCompiler) + "\"";
		}

		if (!RunProcess(configureCmd, pluginDir, result.output, onOutput))
		{
			result.output += "\n[Configure step failed]\n";
			return result;
		}

		const std::string buildCmd =
			cmakeExe + " --build " + cmakeOut +
			" --config " + config.buildType + " --parallel";

		if (!RunProcess(buildCmd, pluginDir, result.output, onOutput))
		{
			result.output += "\n[Build step failed]\n";
			return result;
		}

		result.success = true;
		return result;
	}
}
