module;

#include <iostream>

export module PluginTest;

import PluginContext;
import EditorUI;

struct KeepAlive {
	std::unique_ptr<cp::IWindow> window;
	std::unique_ptr<cp::IDockableWindow> dockWin;
};

extern "C" __declspec(dllexport) void EntryPoint(const cp::PluginContext& ctx) {
	cp::SayHelloFromLoader("PluginTest");

	cp::QtEditorUIFactory factory;
	
	auto win = factory.CreateWindow();
	win->SetTitle("Coucou");
	win->Show();

	auto dock = factory.CreateDockableWindow(win.get());
	dock->MatchSizeToContent();
	dock->Show();

	KeepAlive* ka = new KeepAlive();
	ka->window = std::move(win);
	ka->dockWin = std::move(dock);
}