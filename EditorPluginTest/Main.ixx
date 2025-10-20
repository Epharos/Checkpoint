module;

#include <iostream>

export module PluginTest;

import PluginLoader;

void EntryPoint(const cp::PluginContext& ctx) {
	SayHelloFromLoader("PluginTest");
}