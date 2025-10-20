module;

#include "../pch.hpp"
#include "../macros.hpp"

export module PluginContext;

export namespace cp {
	struct PluginContext {
		uint32_t version;

		EDITOR_API static uint32_t MakeVersion(uint8_t major, uint8_t minor, uint16_t patch) {
			return (static_cast<uint32_t>(major) << 24) | (static_cast<uint32_t>(minor) << 16) | patch;
		}
	};

	export EDITOR_API const void SayHelloFromLoader(std::string_view pluginName) {
		LOG_INFO(MF("Hello from PluginLoader to ", pluginName, "!"));
	}
}