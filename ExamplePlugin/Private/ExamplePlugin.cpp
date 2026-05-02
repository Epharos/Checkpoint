#include <Common/Plugin/PluginAPI.hpp>

extern bool RegisterExamplePluginRuntime(cp::PluginRuntimeContext& _context);
extern void ShutdownExamplePluginRuntime(cp::PluginRuntimeContext& _context);
extern bool RegisterExamplePluginEditor(cp::PluginEditorContext& _context);
extern void ShutdownExamplePluginEditor(cp::PluginEditorContext& _context);

CP_DECLARE_PLUGIN(
	"ExamplePlugin",
	RegisterExamplePluginRuntime,
	ShutdownExamplePluginRuntime,
	RegisterExamplePluginEditor,
	ShutdownExamplePluginEditor
)
