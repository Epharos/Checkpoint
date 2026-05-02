#include "PluginHost.hpp"

#include <algorithm>
#include <cctype>
#include <ranges>
#include <string>

#if defined(CP_PLATFORM_WINDOWS)
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <Windows.h>
#endif

namespace cp
{
	namespace
	{
		[[nodiscard]] std::string ToLower(std::string _value)
		{
			std::ranges::transform(
				_value,
				_value.begin(),
				[](const unsigned char _c) { return static_cast<char>(std::tolower(_c)); }
			);

			return _value;
		}

		[[nodiscard]] bool IsPluginExtension(const std::filesystem::path& _path)
		{
			return ToLower(_path.extension().string()) == ".dll";
		}

#if defined(CP_PLATFORM_WINDOWS)
		[[nodiscard]] std::string FormatLastWin32Error()
		{
			const DWORD error = GetLastError();
			if (error == 0)
			{
				return {};
			}

			LPSTR messageBuffer = nullptr;
			const DWORD messageLength = FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr,
				error,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				reinterpret_cast<LPSTR>(&messageBuffer),
				0,
				nullptr
			);

			std::string message;
			if (messageLength > 0 && messageBuffer != nullptr)
			{
				message.assign(messageBuffer, messageLength);
				LocalFree(messageBuffer);
			}

			return message;
		}

		[[nodiscard]] bool OpenLibrary(const std::filesystem::path& _path, void*& _handle, std::string& _error)
		{
			_handle = static_cast<void*>(LoadLibraryW(_path.c_str()));
			if (_handle == nullptr)
			{
				_error = "LoadLibraryW failed for '" + _path.string() + "': " + FormatLastWin32Error();
				return false;
			}

			return true;
		}

		[[nodiscard]] PluginEntryFn LoadEntryPoint(void* _handle, std::string& _error)
		{
			const FARPROC symbolAddress = GetProcAddress(static_cast<HMODULE>(_handle), PluginEntryPointName.data());
			if (symbolAddress == nullptr)
			{
				_error = "GetProcAddress failed for symbol '" + std::string(PluginEntryPointName) + "': " + FormatLastWin32Error();
				return nullptr;
			}

			return reinterpret_cast<PluginEntryFn>(symbolAddress);
		}

		[[nodiscard]] bool CloseLibrary(void* _handle, std::string& _error)
		{
			if (_handle == nullptr)
			{
				return true;
			}

			if (FreeLibrary(static_cast<HMODULE>(_handle)) == 0)
			{
				_error = "FreeLibrary failed: " + FormatLastWin32Error();
				return false;
			}

			return true;
		}
#else
		[[nodiscard]] bool OpenLibrary(const std::filesystem::path&, void*&, std::string& _error)
		{
			_error = "PluginHost currently supports Windows DLL loading only.";
			return false;
		}

		[[nodiscard]] PluginEntryFn LoadEntryPoint(void*, std::string& _error)
		{
			_error = "PluginHost currently supports Windows DLL loading only.";
			return nullptr;
		}

		[[nodiscard]] bool CloseLibrary(void*, std::string&)
		{
			return true;
		}
#endif
	}

	PluginHost::PluginHost(PluginHostContext _context) : context(_context)
	{
	}

	PluginHost::~PluginHost()
	{
		UnloadAll();
	}

	PluginHost::PluginHost(PluginHost&& _other) noexcept
		: context(_other.context), plugins(std::move(_other.plugins)), lastError(std::move(_other.lastError))
	{
		_other.context = {};
	}

	PluginHost& PluginHost::operator=(PluginHost&& _other) noexcept
	{
		if (this == &_other)
		{
			return *this;
		}

		UnloadAll();

		context = _other.context;
		plugins = std::move(_other.plugins);
		lastError = std::move(_other.lastError);
		_other.context = {};

		return *this;
	}

	bool PluginHost::LoadPlugin(const std::filesystem::path& _path)
	{
		lastError.clear();

		if (!std::filesystem::exists(_path) || !std::filesystem::is_regular_file(_path))
		{
			lastError = "Plugin path does not exist or is not a regular file: " + _path.string();
			return false;
		}

		if (!IsPluginExtension(_path))
		{
			lastError = "Plugin file must have a .dll extension: " + _path.string();
			return false;
		}

		const auto alreadyLoaded =
			std::ranges::find_if(
				plugins,
				[&](const LoadedPlugin& _plugin){ return _plugin.path == _path; }
			);

		if (alreadyLoaded != plugins.end())
		{
			lastError = "Plugin is already loaded: " + _path.string();
			return false;
		}

		void* nativeHandle = nullptr;
		if (!OpenLibrary(_path, nativeHandle, lastError))
		{
			return false;
		}

		const PluginEntryFn entryPoint = LoadEntryPoint(nativeHandle, lastError);
		if (entryPoint == nullptr)
		{
			std::string closeError;
			[[maybe_unused]] bool flag = CloseLibrary(nativeHandle, closeError);
			return false;
		}

		const PluginDescriptor* descriptor = entryPoint();
		if (descriptor == nullptr)
		{
			lastError = "Plugin entry point returned a null descriptor: " + _path.string();
			std::string closeError;
			[[maybe_unused]] bool flag = CloseLibrary(nativeHandle, closeError);
			return false;
		}

		if (descriptor->apiVersion != PluginApiVersion)
		{
			lastError = "Plugin API version mismatch in '" + _path.string() + "' (expected " + std::to_string(PluginApiVersion) + ", got " + std::to_string(descriptor->apiVersion) + ").";
			std::string closeError;
			[[maybe_unused]] bool flag = CloseLibrary(nativeHandle, closeError);
			return false;
		}

		if (descriptor->registerRuntime == nullptr)
		{
			lastError = "Plugin descriptor has no runtime register function: " + _path.string();
			std::string closeError;
			[[maybe_unused]] bool flag = CloseLibrary(nativeHandle, closeError);
			return false;
		}

		if (!descriptor->registerRuntime(context.runtimeContext))
		{
			lastError = "Plugin runtime registration failed: " + _path.string();
			std::string closeError;
			[[maybe_unused]] bool flag = CloseLibrary(nativeHandle, closeError);
			return false;
		}

		bool editorRegistered = false;
		if (context.loadProfile == PluginHostLoadProfile::RuntimeThenEditor && descriptor->registerEditor != nullptr)
		{
			if (!descriptor->registerEditor(context.editorContext))
			{
				lastError = "Plugin editor registration failed: " + _path.string();
				if (descriptor->shutdownRuntime != nullptr)
				{
					descriptor->shutdownRuntime(context.runtimeContext);
				}
				std::string closeError;
				[[maybe_unused]] bool flag = CloseLibrary(nativeHandle, closeError);
				return false;
			}

			editorRegistered = true;
		}

		LoadedPlugin loadedPlugin;
		loadedPlugin.name = descriptor->name != nullptr ? descriptor->name : _path.stem().string();
		loadedPlugin.path = _path;
		loadedPlugin.nativeHandle = nativeHandle;
		loadedPlugin.shutdownRuntime = descriptor->shutdownRuntime;
		loadedPlugin.shutdownEditor = descriptor->shutdownEditor;
		loadedPlugin.runtimeRegistered = true;
		loadedPlugin.editorRegistered = editorRegistered;
		plugins.emplace_back(std::move(loadedPlugin));

		return true;
	}

	size_t PluginHost::LoadPluginsFromDirectory(const std::filesystem::path& _directory, bool _recursive)
	{
		lastError.clear();

		if (!std::filesystem::exists(_directory) || !std::filesystem::is_directory(_directory))
		{
			lastError = "Plugin directory does not exist: " + _directory.string();
			return 0;
		}

		size_t loadedCount = 0;

		const auto loadIfPlugin = [&](const std::filesystem::directory_entry& _entry)
		{
			if (!_entry.is_regular_file() || !IsPluginExtension(_entry.path()))
			{
				return;
			}

			if (LoadPlugin(_entry.path()))
			{
				++loadedCount;
			}
		};

		if (_recursive)
		{
			for (const auto& entry : std::filesystem::recursive_directory_iterator(_directory))
			{
				loadIfPlugin(entry);
			}
		}
		else
		{
			for (const auto& entry : std::filesystem::directory_iterator(_directory))
			{
				loadIfPlugin(entry);
			}
		}

		return loadedCount;
	}

	bool PluginHost::UnloadPlugin(std::string_view _pluginName)
	{
		lastError.clear();

		const auto pluginIterator = std::ranges::find_if(
			plugins,
			[&](const LoadedPlugin& _plugin){ return _plugin.name == _pluginName; }
		);

		if (pluginIterator == plugins.end())
		{
			lastError = "No loaded plugin named '" + std::string(_pluginName) + "'.";
			return false;
		}

		if (pluginIterator->editorRegistered && pluginIterator->shutdownEditor != nullptr)
		{
			pluginIterator->shutdownEditor(context.editorContext);
		}

		if (pluginIterator->runtimeRegistered && pluginIterator->shutdownRuntime != nullptr)
		{
			pluginIterator->shutdownRuntime(context.runtimeContext);
		}

		if (!CloseLibrary(pluginIterator->nativeHandle, lastError))
		{
			return false;
		}

		plugins.erase(pluginIterator);

		return true;
	}

	void PluginHost::UnloadAll()
	{
		lastError.clear();

		for (auto pluginIterator = plugins.rbegin(); pluginIterator != plugins.rend(); ++pluginIterator)
		{
			if (pluginIterator->editorRegistered && pluginIterator->shutdownEditor != nullptr)
			{
				pluginIterator->shutdownEditor(context.editorContext);
			}

			if (pluginIterator->runtimeRegistered && pluginIterator->shutdownRuntime != nullptr)
			{
				pluginIterator->shutdownRuntime(context.runtimeContext);
			}

			std::string closeError;
			if (!CloseLibrary(pluginIterator->nativeHandle, closeError))
			{
				lastError = std::move(closeError);
			}
		}

		plugins.clear();
	}

	bool PluginHost::IsPluginLoaded(std::string_view _pluginName) const
	{
		return std::ranges::any_of(plugins, [&](const LoadedPlugin& _plugin)
		{
			return _plugin.name == _pluginName;
		});
	}

	std::vector<std::string> PluginHost::GetLoadedPluginNames() const
	{
		std::vector<std::string> names;
		names.reserve(plugins.size());

		for (const auto& plugin : plugins)
		{
			names.emplace_back(plugin.name);
		}

		return names;
	}

	const std::string& PluginHost::GetLastError() const
	{
		return lastError;
	}
}
