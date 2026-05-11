#include "../Public/Editor/EditorWorkspace.hpp"

#include <Common/Plugin/ComponentAuthoring.hpp>
#include <Common/Plugin/RenderPassAuthoring.hpp>
#include <Common/Plugin/SystemAuthoring.hpp>
#include <Common/Plugin/PluginHost.hpp>
#include <Common/Plugin/PluginRegistryNames.hpp>

#include <ECS/ECS.hpp>

#include <Rendering/Renderer.hpp>
#include <Rendering/Scene/FrameGraphConfig.hpp>

#include <Runtime/Scene/Scene.hpp>
#include <RuntimeEditorApplication.hpp>

#include <Resources/AssetRegistry.hpp>
#include <Resources/Mesh.hpp>

#include <VulkanRHI.hpp>

#include <filesystem>

namespace cp::editor
{
	namespace
	{
		std::shared_ptr<cp::editorui::IAction> RegisterAction(
			const std::shared_ptr<cp::editorui::IActionRegistry>& _registry,
			cp::editorui::ActionDescriptor _descriptor)
		{
			return _registry->CreateAction(_descriptor);
		}

		cp::editorui::LogLevel ToEditorLogLevel(const cp::LogLevel& _level)
		{
			if (_level.severity >= cp::ILogger::Critical.severity)
			{
				return cp::editorui::LogLevel::Fatal;
			}
			if (_level.severity >= cp::ILogger::Error.severity)
			{
				return cp::editorui::LogLevel::Error;
			}
			if (_level.severity >= cp::ILogger::Warning.severity)
			{
				return cp::editorui::LogLevel::Warning;
			}
			if (_level.severity >= cp::ILogger::Info.severity)
			{
				return cp::editorui::LogLevel::Info;
			}
			if (_level.severity >= cp::ILogger::Debug.severity)
			{
				return cp::editorui::LogLevel::Debug;
			}
			return cp::editorui::LogLevel::Trace;
		}

		cp::editorui::RGBA8 ToEditorColor(const cp::ColorRGBA8& _color)
		{
			return cp::editorui::RGBA8{
				.r = _color.Red(),
				.g = _color.Green(),
				.b = _color.Blue(),
				.a = _color.Alpha()
			};
		}

		cp::editorui::EntityId ToEditorEntityId(const cp::ecs::Entity _entity)
		{
			return (static_cast<uint64_t>(_entity.generation) << 32u) | static_cast<uint64_t>(_entity.index);
		}

		cp::editorui::InspectorField::Value ToEditorInspectorValue(const cp::AuthoringValue& _value)
		{
			if (const bool* boolValue = std::get_if<bool>(&_value))
			{
				return *boolValue;
			}
			if (const int64_t* intValue = std::get_if<int64_t>(&_value))
			{
				return *intValue;
			}
			if (const double* floatValue = std::get_if<double>(&_value))
			{
				return *floatValue;
			}
			if (const std::string* stringValue = std::get_if<std::string>(&_value))
			{
				return *stringValue;
			}
			if (const cp::AuthoringVec3* vec3Value = std::get_if<cp::AuthoringVec3>(&_value))
			{
				return cp::editorui::InspectorField::Vec3{
					.x = vec3Value->x,
					.y = vec3Value->y,
					.z = vec3Value->z
				};
			}

			return std::string{};
		}

		cp::editorui::InspectorField::ValueType ToEditorInspectorValueType(const cp::AuthoringValueType _type)
		{
			switch (_type)
			{
			case cp::AuthoringValueType::Bool:
				return cp::editorui::InspectorField::ValueType::Bool;
			case cp::AuthoringValueType::Int:
				return cp::editorui::InspectorField::ValueType::Int;
			case cp::AuthoringValueType::Float:
				return cp::editorui::InspectorField::ValueType::Float;
			case cp::AuthoringValueType::Vec3:
				return cp::editorui::InspectorField::ValueType::Vec3;
			case cp::AuthoringValueType::String:
			default:
				return cp::editorui::InspectorField::ValueType::String;
			}
		}

		cp::AuthoringValue ToAuthoringValue(const cp::editorui::InspectorField::Value& _value)
		{
			if (const bool* boolValue = std::get_if<bool>(&_value))
			{
				return *boolValue;
			}
			if (const int64_t* intValue = std::get_if<int64_t>(&_value))
			{
				return *intValue;
			}
			if (const double* floatValue = std::get_if<double>(&_value))
			{
				return *floatValue;
			}
			if (const std::string* stringValue = std::get_if<std::string>(&_value))
			{
				return *stringValue;
			}
			if (const cp::editorui::InspectorField::Vec3* vec3Value = std::get_if<cp::editorui::InspectorField::Vec3>(&_value))
			{
				return cp::AuthoringVec3{
					.x = vec3Value->x,
					.y = vec3Value->y,
					.z = vec3Value->z
				};
			}

			return std::string{};
		}

		std::string ToText(const cp::Message& _message)
		{
			std::string output;
			for (const std::shared_ptr<cp::IMessageComponent>& component : _message.components)
			{
				const auto textComponent = std::dynamic_pointer_cast<cp::TextComponent>(component);
				if (!textComponent)
				{
					continue;
				}

				if (!output.empty())
				{
					output += ' ';
				}
				output += textComponent->GetText();
			}

			return output.empty() ? "<empty>" : output;
		}

		class MemorySerializer final : public cp::ISerializer
		{
		public:
			void Write(const std::span<const std::byte> _src) override
			{
				bytes.insert(bytes.end(), _src.begin(), _src.end());
			}

			std::vector<std::byte> bytes;
		};

		class MemoryDeserializer final : public cp::IDeserializer
		{
		public:
			explicit MemoryDeserializer(const std::vector<uint8_t>& _bytes)
				: bytes(_bytes)
			{
			}

			[[nodiscard]] bool Read(const std::span<std::byte> _dst) override
			{
				if (cursor + _dst.size() > bytes.size())
				{
					return false;
				}

				std::memcpy(_dst.data(), bytes.data() + cursor, _dst.size());
				cursor += _dst.size();
				return true;
			}

		private:
			const std::vector<uint8_t>& bytes;
			size_t cursor = 0;
		};

		[[nodiscard]] bool SaveBinaryFile(const std::filesystem::path& _path, const std::vector<std::byte>& _bytes)
		{
			std::ofstream file(_path, std::ios::binary | std::ios::trunc);
			if (!file.is_open())
			{
				return false;
			}

			if (!_bytes.empty())
			{
				file.write(reinterpret_cast<const char*>(_bytes.data()), static_cast<std::streamsize>(_bytes.size()));
			}

			return file.good();
		}

		struct ComponentAuthoringBinding
		{
			std::string registryKey;
			std::unique_ptr<cp::IComponentAuthoring> authoring;
			std::shared_ptr<cp::editorui::IAction> addAction;
			std::shared_ptr<cp::editorui::IAction> removeAction;
		};

		struct SystemAuthoringBinding
		{
			std::string registryKey;
			std::unique_ptr<cp::ISystemAuthoring> authoring;
		};

		struct PassAuthoringBinding
		{
			std::string registryKey;
			std::unique_ptr<cp::IRenderPassAuthoring> authoring;
		};

		struct EditorSceneState
		{
			std::vector<cp::editorui::SceneHierarchyNode> hierarchyNodes;
			std::unordered_map<cp::editorui::EntityId, cp::ecs::Entity> entityLookup;
			std::unordered_map<std::string, size_t> sectionToAuthoring;
			std::optional<cp::editorui::EntityId> selectedEntityId;
			std::optional<cp::editorui::EntityId> contextEntityId;
		};

		[[nodiscard]] bool InsertChildNode(
			std::vector<cp::editorui::SceneHierarchyNode>& _nodes,
			const cp::editorui::EntityId _parentId,
			cp::editorui::SceneHierarchyNode _child)
		{
			for (cp::editorui::SceneHierarchyNode& node : _nodes)
			{
				if (node.entityId == _parentId)
				{
					node.expanded = true;
					node.children.push_back(std::move(_child));
					return true;
				}

				if (InsertChildNode(node.children, _parentId, _child))
				{
					return true;
				}
			}

			return false;
		}

		class EditorConsoleLogger final : public cp::ILogger
		{
		public:
			explicit EditorConsoleLogger(std::function<void(cp::editorui::ConsoleEntry)> _sink)
				: ILogger(nullptr),
				  sink(std::move(_sink))
			{
			}

			void Log(std::string_view _message, cp::LogLevel _logLevel) override
			{
				if (!sink)
				{
					return;
				}

				sink(cp::editorui::ConsoleEntry{
					.level = ToEditorLogLevel(_logLevel),
					.channel = "Editor",
					.message = std::string(_message),
					.timestamp = std::chrono::system_clock::now()
				});
			}

			void Log(const cp::Message& _message, cp::LogLevel _logLevel) override
			{
				if (!sink)
				{
					return;
				}

				sink(cp::editorui::ConsoleEntry{
					.level = ToEditorLogLevel(_logLevel),
					.channel = "Editor",
					.message = ToText(_message),
					.timestamp = std::chrono::system_clock::now()
				});
			}

			void Log(const cp::LogEvent& _event) override
			{
				if (!sink)
				{
					return;
				}

				sink(cp::editorui::ConsoleEntry{
					.level = ToEditorLogLevel(_event.level),
					.channel = std::string(_event.label),
					.message = ToText(_event.message),
					.timestamp = _event.timestamp
				});
			}

			void Spacing(size_t) override
			{
			}

		private:
			std::function<void(cp::editorui::ConsoleEntry)> sink;
		};
	}

	EditorWorkspace::EditorWorkspace(std::shared_ptr<cp::editorui::IEditorUIBackend> _backend)
		: backend(std::move(_backend))
		, mainThreadId(std::this_thread::get_id())
	{
		if (!backend)
		{
			throw std::invalid_argument("EditorWorkspace requires a valid UI backend.");
		}
	}

	EditorWorkspace::~EditorWorkspace()
	{
		if (runtimeApp != nullptr)
		{
			runtimeApp->Stop();
			runtimeApp.reset();
		}

		if (viewportView != nullptr)
		{
			viewportView->SetFrameHandler(nullptr);
			viewportView->SetResizeHandler(nullptr);
		}

		if (renderer != nullptr)
		{
			renderer->ResetFrameGraph();
		}
		renderer.reset();
		rhi.reset();

		if (assetRegistryInitialized)
		{
			cp::AssetRegistry::Instance().Cleanup();
			assetRegistryInitialized = false;
		}

		if (scene != nullptr)
		{
			scene->GetWorld().ClearComponentTypes();
			scene->Clear();
		}

		if (sceneConfigView != nullptr)
		{
			sceneConfigView->SetSelectionChangedHandler(nullptr);
			sceneConfigView->SetParamFieldEditedHandler(nullptr);
			sceneConfigView->SetApplyHandler(nullptr);
		}
		if (inspectorView != nullptr)
		{
			inspectorView->SetFieldEditedHandler(nullptr);
			inspectorView->SetAddComponentMenuHandler(nullptr);
		}
		if (hierarchyView != nullptr)
		{
			hierarchyView->SetEntityActivatedHandler(nullptr);
			hierarchyView->SetContextMenuHandler(nullptr);
		}


		loadSceneDelegate = nullptr;

		if (pluginHost != nullptr)
		{
			pluginHost->UnloadAll();
			pluginHost.reset();
		}

		registryManager.reset();
	}

	void EditorWorkspace::AppendConsoleEntry(cp::editorui::ConsoleEntry _entry)
	{
		if (std::this_thread::get_id() != mainThreadId)
		{
			std::lock_guard lock(pendingEntriesMutex);
			pendingConsoleEntries.push_back(std::move(_entry));
			return;
		}

		consoleEntries.push_back(std::move(_entry));
		if (consoleEntries.size() > 2000)
		{
			consoleEntries.erase(consoleEntries.begin(), consoleEntries.begin() + static_cast<long long>(consoleEntries.size() - 2000));
		}

		if (consoleView)
		{
			consoleView->SetEntries(consoleEntries);
		}
	}

	void EditorWorkspace::BuildDefaultLayout(const WorkspaceConfig& _config)
	{
		registryManager = std::make_unique<cp::RegistryManager>();
		registryManager->GetOrCreate<cp::IRenderPass>(std::string(cp::RenderPassRegistryName));
		registryManager->GetOrCreate<cp::ecs::ISystem>(std::string(cp::EcsSystemRegistryName));
		registryManager->GetOrCreate<cp::ecs::IComponentRegistrar>(std::string(cp::EcsComponentRegistryName));
		registryManager->GetOrCreate<cp::IComponentAuthoring>(std::string(cp::EcsComponentAuthoringRegistryName));
		registryManager->GetOrCreate<cp::ISystemAuthoring>(std::string(cp::EcsSystemAuthoringRegistryName));
		registryManager->GetOrCreate<cp::IRenderPassAuthoring>(std::string(cp::RenderPassAuthoringRegistryName));
		scene = std::make_unique<cp::runtime::Scene>();
		scene->Initialize("NewScene");

		cp::editorui::ApplicationWindowConfig windowConfig;
		windowConfig.title = _config.windowTitle;

		window = backend->CreateApplicationWindow(windowConfig);
		const auto theme = backend->CreateTheme("CheckpointDark");
		theme->SetColor(cp::editorui::ThemeColorRole::WindowBackground, ToEditorColor(cp::ColorRGBA8(33, 36, 44, 255)));
		theme->SetColor(cp::editorui::ThemeColorRole::PanelBackground, ToEditorColor(cp::ColorRGBA8(41, 46, 56, 255)));
		theme->SetColor(cp::editorui::ThemeColorRole::PanelBorder, ToEditorColor(cp::ColorRGBA8(62, 69, 82, 255)));
		theme->SetColor(cp::editorui::ThemeColorRole::TextPrimary, ToEditorColor(cp::ColorRGBA8(224, 228, 235, 255)));
		theme->SetColor(cp::editorui::ThemeColorRole::TextSecondary, ToEditorColor(cp::ColorRGBA8(167, 176, 189, 255)));
		theme->SetColor(cp::editorui::ThemeColorRole::Accent, ToEditorColor(cp::ColorRGBA8(86, 146, 255, 255)));
		theme->SetColor(cp::editorui::ThemeColorRole::Selection, ToEditorColor(cp::ColorRGBA8(68, 105, 168, 255)));
		theme->SetColor(cp::editorui::ThemeColorRole::Warning, ToEditorColor(cp::ColorRGBA8(234, 179, 8, 255)));
		theme->SetColor(cp::editorui::ThemeColorRole::Error, ToEditorColor(cp::ColorRGBA8(239, 68, 68, 255)));
		window->SetTheme(theme);

		actions = backend->CreateActionRegistry();
		commandStack = backend->CreateCommandStack();
		dockHost = window->GetDockHost();

		const auto openProjectAction = RegisterAction(actions, { "file.openProject", "Open Project" });
		const auto loadSceneAction = RegisterAction(actions, { "file.loadScene", "Load Scene" });
		const auto saveSceneAction = RegisterAction(actions, { "file.saveScene", "Save Scene" });
		const auto undoAction = RegisterAction(actions, { "edit.undo", "Undo", "", "", cp::editorui::Shortcut{ "Ctrl+Z" } });
		const auto redoAction = RegisterAction(actions, { "edit.redo", "Redo", "", "", cp::editorui::Shortcut{ "Ctrl+Y" } });
		const auto buildAction = RegisterAction(actions, { "project.build", "Build", "Compile the project." });
		runAction = RegisterAction(actions, { "project.run", "Run", "Launch runtime." });
		stopAction = RegisterAction(actions, { "project.stop", "Stop", "Stop runtime." });
		stopAction->SetEnabled(false);
		const auto createEntityAction = RegisterAction(actions, { "hierarchy.createEntity", "Create Entity" });
		const auto toolTranslateAction = RegisterAction(actions, { "viewport.tool.translate", "Translate", "", "", std::nullopt, true, true });
		const auto toolRotateAction = RegisterAction(actions, { "viewport.tool.rotate", "Rotate", "", "", std::nullopt, true, false });
		const auto toolScaleAction = RegisterAction(actions, { "viewport.tool.scale", "Scale", "", "", std::nullopt, true, false });

		const auto menuBar = window->GetMenuBar();
		const auto fileMenu = menuBar->AddMenu("file", "File");
		fileMenu->AddAction(openProjectAction);
		fileMenu->AddAction(loadSceneAction);
		fileMenu->AddAction(saveSceneAction);

		const auto editMenu = menuBar->AddMenu("edit", "Edit");
		editMenu->AddAction(undoAction);
		editMenu->AddAction(redoAction);
		editMenu->AddAction(createEntityAction);

		const auto mainToolBar = window->AddToolBar("main");
		mainToolBar->AddAction(buildAction);
		mainToolBar->AddAction(runAction);
		mainToolBar->AddAction(stopAction);

		const auto hierarchyPanel = dockHost->CreatePanel({ "hierarchy", "Hierarchy" });
		hierarchyView = backend->CreateSceneHierarchyView("sceneHierarchy");
		hierarchyPanel->SetContent(hierarchyView);

		const auto assetsPanel = dockHost->CreatePanel({ "assets", "Assets" });
		assetView = backend->CreateAssetExplorerView("assetExplorer");
		assetView->SetProjectRoot(_config.projectRootPath);
		assetsPanel->SetContent(assetView);

		const auto consolePanel = dockHost->CreatePanel({ "console", "Console" });
		consoleView = backend->CreateConsoleView("console");
		consoleView->SetMinLevelFilter(cp::editorui::LogLevel::Trace);
		consolePanel->SetContent(consoleView);

		const auto inspectorPanel = dockHost->CreatePanel({ "inspector", "Inspector" });
		inspectorView = backend->CreateInspectorView("inspector");
		inspectorPanel->SetContent(inspectorView);

		const auto sceneConfigPanel = dockHost->CreatePanel({ "sceneConfig", "Scene Config" });
		sceneConfigView = backend->CreateSceneConfigView("sceneConfig");
		sceneConfigPanel->SetContent(sceneConfigView);

		const auto viewportPanel = dockHost->CreatePanel({ "viewport", "Viewport", false, true });
		viewportView = backend->CreateViewportWidget({ "mainViewport" });
		viewportPanel->SetContent(viewportView);

		const auto viewportToolBar = viewportPanel->AddToolBar("viewport.tools");
		viewportToolBar->AddAction(toolTranslateAction);
		viewportToolBar->AddAction(toolRotateAction);
		viewportToolBar->AddAction(toolScaleAction);

		const auto consoleLogger = std::make_shared<cp::ConsoleLogger>(std::make_shared<cp::ConsoleVisitor>());
		const auto editorConsoleLogger = std::make_shared<EditorConsoleLogger>(
			[this](cp::editorui::ConsoleEntry _entry)
			{
				AppendConsoleEntry(std::move(_entry));
			});

		const auto compositeLogger = std::make_shared<cp::CompositeLogger>();
		compositeLogger->AddLogger(consoleLogger);
		compositeLogger->AddLogger(editorConsoleLogger);
		logger = compositeLogger;

		logger->Log(CP_LOG_EVENT(cp::ILogger::Info, "Editor", cp::Message::Create("Editor workspace bootstrapped")));

		cp::PluginHostContext pluginHostContext
		{
			.loadProfile = cp::PluginHostLoadProfile::RuntimeThenEditor,
			.runtimeContext = cp::PluginRuntimeContext{
				.mainLogger = logger.get(),
				.registryManager = registryManager.get(),
				.assetRegistry = &cp::AssetRegistry::Instance()
			},
			.editorContext = cp::PluginEditorContext{
				.mainLogger = logger.get(),
				.registryManager = registryManager.get(),
				.assetRegistry = &cp::AssetRegistry::Instance()
			}
		};
		pluginHost = std::make_unique<cp::PluginHost>(pluginHostContext);

		const std::filesystem::path executableDirectory = !_config.executablePath.empty()
			                                                  ? _config.executablePath.parent_path()
			                                                  : std::filesystem::current_path();
		const std::filesystem::path pluginsDirectory = executableDirectory / "Plugins";

		if (std::filesystem::exists(pluginsDirectory) && std::filesystem::is_directory(pluginsDirectory))
		{
			const size_t loadedPluginCount = pluginHost->LoadPluginsFromDirectory(pluginsDirectory);
			if (loadedPluginCount > 0)
			{
				logger->Log(CP_LOG_EVENT(
					cp::ILogger::Info,
					"Plugins",
					cp::Message::Create("Loaded {} plugin(s) from {}", loadedPluginCount, pluginsDirectory.string())
				));
			}
			else
			{
				const std::string& pluginError = pluginHost->GetLastError();
				if (!pluginError.empty())
				{
					logger->Log(CP_LOG_EVENT(
						cp::ILogger::Warning,
						"Plugins",
						cp::Message::Create("No plugin loaded from {} ({})", pluginsDirectory.string(), pluginError)
					));
				}
				else
				{
					logger->Log(CP_LOG_EVENT(
						cp::ILogger::Info,
						"Plugins",
						cp::Message::Create("No plugins found in {}", pluginsDirectory.string())
					));
				}
			}
		}
		else
		{
			logger->Log(CP_LOG_EVENT(
				cp::ILogger::Info,
				"Plugins",
				cp::Message::Create("Plugin directory not found: {}", pluginsDirectory.string())
			));
		}

		if (const cp::Registry<cp::ecs::IComponentRegistrar>* componentRegistry =
			registryManager->Find<cp::ecs::IComponentRegistrar>(cp::EcsComponentRegistryName))
		{
			for (const std::string& registrarName : componentRegistry->Names())
			{
				const std::unique_ptr<cp::ecs::IComponentRegistrar> registrar = componentRegistry->Create(registrarName);
				registrar->Register(scene->GetWorld());
			}
		}

		scene->GetWorld().SetRequiredPlugins(pluginHost->GetLoadedPluginNames());
		const auto componentAuthoringBindings = std::make_shared<std::vector<ComponentAuthoringBinding>>();
		if (const cp::Registry<cp::IComponentAuthoring>* componentAuthoringRegistry =
			registryManager->Find<cp::IComponentAuthoring>(cp::EcsComponentAuthoringRegistryName))
		{
			for (const std::string& authoringKey : componentAuthoringRegistry->Names())
			{
				ComponentAuthoringBinding binding;
				binding.registryKey = authoringKey;
				binding.authoring = componentAuthoringRegistry->Create(authoringKey);
				binding.addAction = RegisterAction(actions, {
					"hierarchy.addComponent." + authoringKey,
					"Add " + std::string(binding.authoring->Name())
				});
				binding.removeAction = RegisterAction(actions, {
					"hierarchy.removeComponent." + authoringKey,
					"Remove " + std::string(binding.authoring->Name())
				});
				componentAuthoringBindings->push_back(std::move(binding));
			}
		}

		const auto systemAuthoringBindings = std::make_shared<std::vector<SystemAuthoringBinding>>();
		if (const cp::Registry<cp::ISystemAuthoring>* sysAuthoringReg =
			registryManager->Find<cp::ISystemAuthoring>(cp::EcsSystemAuthoringRegistryName))
		{
			for (const std::string& key : sysAuthoringReg->Names())
			{
				SystemAuthoringBinding binding;
				binding.registryKey = key;
				binding.authoring = sysAuthoringReg->Create(key);
				systemAuthoringBindings->push_back(std::move(binding));
			}
		}

		const auto passAuthoringBindings = std::make_shared<std::vector<PassAuthoringBinding>>();
		if (const cp::Registry<cp::IRenderPassAuthoring>* passAuthoringReg =
			registryManager->Find<cp::IRenderPassAuthoring>(cp::RenderPassAuthoringRegistryName))
		{
			for (const std::string& key : passAuthoringReg->Names())
			{
				PassAuthoringBinding binding;
				binding.registryKey = key;
				binding.authoring = passAuthoringReg->Create(key);
				passAuthoringBindings->push_back(std::move(binding));
			}
		}

		undoAction->SetEnabled(false);
		redoAction->SetEnabled(false);
		undoAction->SetTriggeredHandler([this, undoAction, redoAction]()
		{
			commandStack->Undo();
			undoAction->SetEnabled(commandStack->CanUndo());
			redoAction->SetEnabled(commandStack->CanRedo());
		});
		redoAction->SetTriggeredHandler([this, undoAction, redoAction]()
		{
			commandStack->Redo();
			undoAction->SetEnabled(commandStack->CanUndo());
			redoAction->SetEnabled(commandStack->CanRedo());
		});

		openProjectAction->SetTriggeredHandler([this, _config]()
		{
			assetView->SetProjectRoot(_config.projectRootPath);
			logger->Log(CP_LOG_EVENT(cp::ILogger::Info, "Assets", cp::Message::Create("Project root set to '{}'", _config.projectRootPath.string())));
		});
		buildAction->SetTriggeredHandler([this]()
		{
			logger->Log(CP_LOG_EVENT(cp::ILogger::Info, "Build", cp::Message::Create("Build requested from editor")));
		});
		runAction->SetTriggeredHandler([this, _config]()
		{
			if (runtimeApp != nullptr)
			{
				return;
			}

			// Serialize the current scene to a backup file
			runtimeBackupPath = (!_config.projectRootPath.empty()
				? _config.projectRootPath
				: std::filesystem::current_path()) / ".runtime_backup.scene";

			MemorySerializer backupSerializer;
			if (!scene->SerializeTo(backupSerializer))
			{
				logger->Log(CP_LOG_EVENT(cp::ILogger::Error, "Run",
					cp::Message::Create("Failed to serialize scene for runtime backup")));
				return;
			}

			if (!SaveBinaryFile(runtimeBackupPath, backupSerializer.bytes))
			{
				logger->Log(CP_LOG_EVENT(cp::ILogger::Error, "Run",
					cp::Message::Create("Failed to save runtime backup to '{}'", runtimeBackupPath.string())));
				return;
			}

			// Deserialize a fresh copy of the scene for the runtime
			auto runtimeScene = std::make_unique<cp::runtime::Scene>();
			if (const cp::Registry<cp::ecs::IComponentRegistrar>* componentRegistry =
				registryManager->Find<cp::ecs::IComponentRegistrar>(cp::EcsComponentRegistryName))
			{
				for (const std::string& registrarName : componentRegistry->Names())
				{
					const std::unique_ptr<cp::ecs::IComponentRegistrar> registrar = componentRegistry->Create(registrarName);
					registrar->Register(runtimeScene->GetWorld());
				}
			}

			const std::vector<uint8_t> runtimeBytes(
				reinterpret_cast<const uint8_t*>(backupSerializer.bytes.data()),
				reinterpret_cast<const uint8_t*>(backupSerializer.bytes.data() + backupSerializer.bytes.size())
			);
			MemoryDeserializer runtimeDeserializer(runtimeBytes);
			if (!runtimeScene->DeserializeFrom(runtimeDeserializer))
			{
				logger->Log(CP_LOG_EVENT(cp::ILogger::Error, "Run",
					cp::Message::Create("Failed to deserialize runtime scene copy")));
				return;
			}

			runtimeApp = std::make_unique<cp::runtime::RuntimeEditorApplication>(
				*rhi, logger, std::move(runtimeScene), *registryManager
			);
			runtimeApp->SetOnStoppedCallback([this]()
			{
				// Called from runtime thread — IsRunning() will be false,
				// the frame handler will detect this and call OnRuntimeStopped() on the main thread.
			});
			runtimeApp->Start();

			runAction->SetEnabled(false);
			stopAction->SetEnabled(true);
			logger->Log(CP_LOG_EVENT(cp::ILogger::Info, "Run", cp::Message::Create("Runtime started")));
		});
		stopAction->SetTriggeredHandler([this]()
		{
			if (runtimeApp == nullptr)
			{
				return;
			}
			OnRuntimeStopped();
		});

		auto setViewportTool = [viewport = viewportView, toolTranslateAction, toolRotateAction, toolScaleAction](const cp::editorui::ViewportTool _tool)
		{
			toolTranslateAction->SetChecked(_tool == cp::editorui::ViewportTool::Translate);
			toolRotateAction->SetChecked(_tool == cp::editorui::ViewportTool::Rotate);
			toolScaleAction->SetChecked(_tool == cp::editorui::ViewportTool::Scale);
			viewport->SetActiveTool(_tool);
		};
		toolTranslateAction->SetTriggeredHandler([setViewportTool]() { setViewportTool(cp::editorui::ViewportTool::Translate); });
		toolRotateAction->SetTriggeredHandler([setViewportTool]() { setViewportTool(cp::editorui::ViewportTool::Rotate); });
		toolScaleAction->SetTriggeredHandler([setViewportTool]() { setViewportTool(cp::editorui::ViewportTool::Scale); });

		const auto sceneState = std::make_shared<EditorSceneState>();
		const auto activeScenePath = std::make_shared<std::filesystem::path>(
			!_config.projectRootPath.empty()
				? _config.projectRootPath / "Scene.scene"
				: std::filesystem::current_path() / "Scene.scene");

		const auto refreshHierarchyView = [this, sceneState]()
		{
			hierarchyView->SetNodes(sceneState->hierarchyNodes);
			hierarchyView->SetSelectedEntity(sceneState->selectedEntityId);
		};

		const auto refreshInspectorView = [this, sceneState, _config, componentAuthoringBindings]()
		{
			std::vector<cp::editorui::InspectorSection> sections;
			sceneState->sectionToAuthoring.clear();

			sections.push_back({
				.id = "workspace",
				.title = "Workspace",
				.fields = {
					{
						.id = "projectRoot",
						.label = "Project Root",
						.valueType = cp::editorui::InspectorField::ValueType::String,
						.value = _config.projectRootPath.string(),
						.readOnly = true
					},
					{
						.id = "viewportTool",
						.label = "Viewport Tool",
						.valueType = cp::editorui::InspectorField::ValueType::String,
						.value = std::string("Translate"),
						.readOnly = true
					}
				}
			});

			if (sceneState->selectedEntityId.has_value())
			{
				const auto it = sceneState->entityLookup.find(sceneState->selectedEntityId.value());
				if (it != sceneState->entityLookup.end())
				{
					const cp::ecs::Entity entity = it->second;
					sections.push_back({
						.id = "entity",
						.title = "Entity",
						.fields = {
							{
								.id = "index",
								.label = "Index",
								.valueType = cp::editorui::InspectorField::ValueType::Int,
								.value = static_cast<int64_t>(entity.index),
								.readOnly = true
							},
							{
								.id = "generation",
								.label = "Generation",
								.valueType = cp::editorui::InspectorField::ValueType::Int,
								.value = static_cast<int64_t>(entity.generation),
								.readOnly = true
							},
							{
								.id = "alive",
								.label = "Alive",
								.valueType = cp::editorui::InspectorField::ValueType::Bool,
								.value = scene->GetWorld().IsAlive(entity),
								.readOnly = true
							}
						}
					});

					for (size_t authoringIndex = 0; authoringIndex < componentAuthoringBindings->size(); ++authoringIndex)
					{
						const ComponentAuthoringBinding& binding = componentAuthoringBindings->at(authoringIndex);
						if (!binding.authoring->HasComponent(scene->GetWorld(), entity))
						{
							continue;
						}

						const std::vector<cp::AuthoringSectionDescriptor> authoredSections =
							binding.authoring->BuildSections(scene->GetWorld(), entity);
						for (const cp::AuthoringSectionDescriptor& authoredSection : authoredSections)
						{
							cp::editorui::InspectorSection inspectorSection;
							inspectorSection.id = authoredSection.id;
							inspectorSection.title = authoredSection.title;
							sceneState->sectionToAuthoring[authoredSection.id] = authoringIndex;

							for (const cp::AuthoringFieldDescriptor& authoredField : authoredSection.fields)
							{
								inspectorSection.fields.push_back({
									.id = authoredField.id,
									.label = authoredField.label,
									.valueType = ToEditorInspectorValueType(authoredField.valueType),
									.inputType = static_cast<cp::editorui::InspectorField::InputType>(authoredField.inputType),
									.value = ToEditorInspectorValue(authoredField.value),
									.readOnly = authoredField.readOnly
								});
							}

							sections.push_back(std::move(inspectorSection));
						}
					}
				}
			}

			inspectorView->SetSections(std::move(sections));
		};

		const auto refreshSceneConfigView = [this]()
		{
			std::vector<cp::editorui::ISceneConfigView::RegistryEntry> passEntries;
			const auto& activePasses = scene->GetActivePassNames();
			if (const auto* passReg = registryManager->Find<cp::IRenderPass>(cp::RenderPassRegistryName))
			{
				for (const auto& name : passReg->Names())
				{
					const bool enabled = std::find(activePasses.begin(), activePasses.end(), name) != activePasses.end();
					passEntries.push_back({ name, "", enabled });
				}
			}
			sceneConfigView->SetRenderPassEntries(std::move(passEntries));

			std::vector<cp::editorui::ISceneConfigView::RegistryEntry> systemEntries;
			const auto& enabledSystems = scene->GetEnabledSystemGuids();
			if (const auto* sysReg = registryManager->Find<cp::ecs::ISystem>(cp::EcsSystemRegistryName))
			{
				for (const auto& registryKey : sysReg->Names())
				{
					const bool enabled = std::find(enabledSystems.begin(), enabledSystems.end(), registryKey) != enabledSystems.end();
					auto instance = sysReg->Create(registryKey);
					std::string displayName = instance ? std::string(instance->Name()) : registryKey;
					systemEntries.push_back({ std::move(displayName), registryKey, enabled });
				}
			}
			sceneConfigView->SetSystemEntries(std::move(systemEntries));
		};

		auto toInspectorSections = [](const std::vector<cp::AuthoringSectionDescriptor>& _authored)
		{
			std::vector<cp::editorui::InspectorSection> result;
			result.reserve(_authored.size());
			for (const auto& s : _authored)
			{
				cp::editorui::InspectorSection section;
				section.id = s.id;
				section.title = s.title;
				for (const auto& f : s.fields)
				{
					cp::editorui::InspectorField field;
					field.id = f.id;
					field.label = f.label;
					field.valueType = ToEditorInspectorValueType(f.valueType);
					field.inputType = (f.inputType == cp::AuthoringInputType::FilePath)
						? cp::editorui::InspectorField::InputType::FilePath
						: cp::editorui::InspectorField::InputType::Default;
					field.value = ToEditorInspectorValue(f.value);
					field.readOnly = f.readOnly;
					section.fields.push_back(std::move(field));
				}
				result.push_back(std::move(section));
			}
			return result;
		};

		sceneConfigView->SetSelectionChangedHandler([this, systemAuthoringBindings, passAuthoringBindings, toInspectorSections](
			std::string_view _key, bool _isPass)
		{
			if (_isPass)
			{
				const auto it = std::find_if(passAuthoringBindings->begin(), passAuthoringBindings->end(),
					[&](const PassAuthoringBinding& b) { return b.registryKey == _key; });

				if (it == passAuthoringBindings->end())
				{
					sceneConfigView->ClearParamSections();
					return;
				}

				cp::IRenderPass* pass = renderer ? renderer->FindActivePass(_key) : nullptr;

				if (!pass)
				{
					sceneConfigView->ClearParamSections();
					return;
				}

				sceneConfigView->SetParamSections(it->authoring->Name(),
					toInspectorSections(it->authoring->BuildSections(*pass)));
			}
			else
			{
				const auto it = std::find_if(systemAuthoringBindings->begin(), systemAuthoringBindings->end(),
					[&](const SystemAuthoringBinding& b) { return b.registryKey == _key; });

				if (it == systemAuthoringBindings->end())
				{
					sceneConfigView->ClearParamSections();
					return;
				}

				cp::ecs::ISystem* system = scene->FindActiveSystem(_key);

				if (!system)
				{
					sceneConfigView->ClearParamSections();
					return;
				}

				sceneConfigView->SetParamSections(it->authoring->Name(),
					toInspectorSections(it->authoring->BuildSections(*system)));
			}
		});

		sceneConfigView->SetParamFieldEditedHandler([this, systemAuthoringBindings, passAuthoringBindings, toInspectorSections](
			std::string_view _key, bool _isPass,
			std::string_view _sectionId,
			std::string_view _fieldId,
			const cp::editorui::InspectorField::Value& _value)
		{
			std::string error;
			if (_isPass)
			{
				const auto it = std::find_if(passAuthoringBindings->begin(), passAuthoringBindings->end(),
					[&](const PassAuthoringBinding& b) { return b.registryKey == _key; });
				if (it == passAuthoringBindings->end()) return;
				cp::IRenderPass* pass = renderer ? renderer->FindActivePass(_key) : nullptr;
				if (!pass)
				{
					logger->Log(CP_LOG_EVENT(cp::ILogger::Warning, "SceneConfig",
						cp::Message::Create("Pass '{}' has no active instance: cannot apply field '{}'", _key, _fieldId)));
					return;
				}
				if (!it->authoring->ApplyValue(*pass, _fieldId, ToAuthoringValue(_value), error))
				{
					logger->Log(CP_LOG_EVENT(cp::ILogger::Warning, "SceneConfig",
						cp::Message::Create("Pass param '{}': {}", _fieldId, error)));
				}
				// Snapshot the updated pass state into the scene for persistence
				{
					MemorySerializer blobSerializer;
					pass->Serialize(blobSerializer);
					scene->SetPassBlob(std::string(_key), std::move(blobSerializer.bytes));
				}
				// Refresh params panel from the live instance
				sceneConfigView->SetParamSections(it->authoring->Name(),
					toInspectorSections(it->authoring->BuildSections(*pass)));
			}
			else
			{
				const auto it = std::find_if(systemAuthoringBindings->begin(), systemAuthoringBindings->end(),
					[&](const SystemAuthoringBinding& b) { return b.registryKey == _key; });

				if (it == systemAuthoringBindings->end()) return;


				cp::ecs::ISystem* system = scene->FindActiveSystem(_key);
				if (!system)
				{
					logger->Log(CP_LOG_EVENT(cp::ILogger::Warning, "SceneConfig",
						cp::Message::Create("System '{}' has no active instance: cannot apply field '{}'", _key, _fieldId)));
					return;
				}

				if (!it->authoring->ApplyValue(*system, _fieldId, ToAuthoringValue(_value), error))
				{
					logger->Log(CP_LOG_EVENT(cp::ILogger::Warning, "SceneConfig",
						cp::Message::Create("System param '{}': {}", _fieldId, error)));
				}

				sceneConfigView->SetParamSections(it->authoring->Name(),
					toInspectorSections(it->authoring->BuildSections(*system)));
			}
		});

		sceneConfigView->SetApplyHandler([this](
			std::vector<std::string> _passNames,
			std::vector<std::string> _systemNames)
		{
			const auto passCount   = _passNames.size();
			const auto systemCount = _systemNames.size();
			scene->SetActivePassNames(std::move(_passNames));
			scene->SetEnabledSystemGuids(std::move(_systemNames));

			if (const cp::Registry<cp::ecs::ISystem>* sysReg =
				registryManager->Find<cp::ecs::ISystem>(cp::EcsSystemRegistryName))
			{
				scene->InitializeSystems(*sysReg);
			}

			if (renderer)
			{
				for (const auto& [typeName, blob] : renderer->SnapshotPassBlobs())
					scene->SetPassBlob(typeName, blob);

				renderer->SetPendingPassBlobs(scene->GetPassBlobs());
				cp::rendering::ApplyFrameGraphConfigToRenderer(*renderer, scene->GetActivePassNames());
			}
			logger->Log(CP_LOG_EVENT(cp::ILogger::Info, "Scene",
				cp::Message::Create("Applied scene config: {} pass(es), {} system(s)", passCount, systemCount)));
		});

		refreshSceneConfigView();

		const auto selectedEntityResolver = [sceneState]() -> std::optional<cp::ecs::Entity>
		{
			const std::optional<cp::editorui::EntityId> targetId =
				sceneState->contextEntityId.has_value()
					? sceneState->contextEntityId
					: sceneState->selectedEntityId;

			if (!targetId.has_value())
			{
				return std::nullopt;
			}

			const auto it = sceneState->entityLookup.find(targetId.value());
			if (it == sceneState->entityLookup.end())
			{
				return std::nullopt;
			}
			return it->second;
		};

		inspectorView->SetFieldEditedHandler([this, sceneState, selectedEntityResolver, componentAuthoringBindings, refreshInspectorView](
			const std::string_view _sectionId,
			const std::string_view _fieldId,
			const cp::editorui::InspectorField::Value& _value)
		{
			const std::optional<cp::ecs::Entity> selectedEntity = selectedEntityResolver();
			if (!selectedEntity.has_value())
			{
				return;
			}

			const auto authoringIt = sceneState->sectionToAuthoring.find(std::string(_sectionId));
			if (authoringIt == sceneState->sectionToAuthoring.end())
			{
				return;
			}

			const size_t authoringIndex = authoringIt->second;
			if (authoringIndex >= componentAuthoringBindings->size())
			{
				return;
			}

			ComponentAuthoringBinding& binding = componentAuthoringBindings->at(authoringIndex);
			std::string error;
			if (!binding.authoring->ApplyValue(scene->GetWorld(), selectedEntity.value(), _fieldId, ToAuthoringValue(_value), error))
			{
				logger->Log(CP_LOG_EVENT(
					cp::ILogger::Warning,
					"Inspector",
					cp::Message::Create("Failed to apply {}.{}: {}", _sectionId, _fieldId, error)
				));
			}

			refreshInspectorView();
		});

		inspectorView->SetAddComponentMenuHandler([this, sceneState, selectedEntityResolver, componentAuthoringBindings]() -> std::shared_ptr<cp::editorui::IContextMenu>
		{
			const auto menu = backend->CreateContextMenu("inspector.addComponent");
			const std::optional<cp::ecs::Entity> selectedEntity = selectedEntityResolver();
			
			if (selectedEntity.has_value() && !componentAuthoringBindings->empty())
			{
				for (ComponentAuthoringBinding& binding : *componentAuthoringBindings)
				{
					const bool hasComponent = binding.authoring->HasComponent(scene->GetWorld(), selectedEntity.value());
					binding.addAction->SetEnabled(!hasComponent);
					menu->AddAction(binding.addAction);
				}
			}
			
			return menu;
		});

		const auto loadScene = [this, sceneState, activeScenePath, refreshHierarchyView, refreshInspectorView, refreshSceneConfigView](const std::filesystem::path& _scenePath) -> bool
		{
			const std::vector<uint8_t> sceneBytes = LoadBinaryFile(_scenePath);
			if (sceneBytes.empty())
			{
				logger->Log(CP_LOG_EVENT(cp::ILogger::Warning, "Scene", cp::Message::Create("Scene file is empty: {}", _scenePath.string())));
				return false;
			}

			MemoryDeserializer deserializer(sceneBytes);
			if (!scene->DeserializeFrom(deserializer))
			{
				logger->Log(CP_LOG_EVENT(cp::ILogger::Warning, "Scene", cp::Message::Create("Failed to deserialize scene '{}'", _scenePath.string())));
				return false;
			}

			if (renderer)
			{
				renderer->SetPendingPassBlobs(scene->GetPassBlobs());
				cp::rendering::ApplyFrameGraphConfigToRenderer(*renderer, scene->GetActivePassNames());
			}

			if (const cp::Registry<cp::ecs::ISystem>* sysReg =
				registryManager->Find<cp::ecs::ISystem>(cp::EcsSystemRegistryName))
			{
				scene->InitializeSystems(*sysReg);
			}

			const std::vector<std::string> loadedPluginNames = pluginHost != nullptr
				                                               ? pluginHost->GetLoadedPluginNames()
				                                               : std::vector<std::string>{};
			for (const std::string& requiredPlugin : scene->GetWorld().GetRequiredPlugins())
			{
				const bool isLoaded = std::find(loadedPluginNames.begin(), loadedPluginNames.end(), requiredPlugin) != loadedPluginNames.end();
				if (!isLoaded)
				{
					logger->Log(CP_LOG_EVENT(cp::ILogger::Warning, "Scene", cp::Message::Create("Scene requires missing plugin '{}'", requiredPlugin)));
				}
			}

			sceneState->hierarchyNodes.clear();
			sceneState->entityLookup.clear();

			std::vector<cp::ecs::Entity> entities = scene->GetWorld().GetAliveEntities();
			std::sort(entities.begin(), entities.end(), [](const cp::ecs::Entity& _lhs, const cp::ecs::Entity& _rhs)
			{
				return _lhs.index < _rhs.index;
			});

			for (const cp::ecs::Entity entity : entities)
			{
				const cp::editorui::EntityId id = ToEditorEntityId(entity);
				sceneState->entityLookup[id] = entity;
				sceneState->hierarchyNodes.push_back({
					.entityId = id,
					.label = "Entity " + std::to_string(entity.index),
					.expanded = false
				});
			}

			sceneState->selectedEntityId = sceneState->hierarchyNodes.empty()
				                               ? std::optional<cp::editorui::EntityId>{}
				                               : std::optional<cp::editorui::EntityId>{ sceneState->hierarchyNodes.front().entityId };
			sceneState->contextEntityId.reset();
			*activeScenePath = _scenePath;
			refreshHierarchyView();
			refreshInspectorView();
			refreshSceneConfigView();
			logger->Log(CP_LOG_EVENT(cp::ILogger::Info, "Scene", cp::Message::Create("Loaded scene '{}'", _scenePath.string())));
			return true;
		};

		loadSceneDelegate = loadScene;

		const auto saveScene = [this, activeScenePath]() -> bool
		{
			if (renderer)
			{
				for (const auto& [typeName, blob] : renderer->SnapshotPassBlobs())
					scene->SetPassBlob(typeName, blob);
			}

			MemorySerializer serializer;
			if (!scene->SerializeTo(serializer))
			{
				logger->Log(CP_LOG_EVENT(cp::ILogger::Warning, "Scene", cp::Message::Create("Failed to serialize scene")));
				return false;
			}

			if (!SaveBinaryFile(*activeScenePath, serializer.bytes))
			{
				logger->Log(CP_LOG_EVENT(cp::ILogger::Warning, "Scene", cp::Message::Create("Failed to save scene '{}'", activeScenePath->string())));
				return false;
			}

			logger->Log(CP_LOG_EVENT(cp::ILogger::Info, "Scene", cp::Message::Create("Saved scene '{}'", activeScenePath->string())));
			return true;
		};

		const auto createEntity = [this, sceneState, refreshHierarchyView, refreshInspectorView]()
		{
			const cp::ecs::Entity entity = scene->GetWorld().CreateEntity();
			const cp::editorui::EntityId entityId = ToEditorEntityId(entity);

			sceneState->entityLookup[entityId] = entity;
			cp::editorui::SceneHierarchyNode node{
				.entityId = entityId,
				.label = "Entity " + std::to_string(entity.index),
				.expanded = false
			};

			const std::optional<cp::editorui::EntityId> parentId =
				sceneState->contextEntityId.has_value()
					? sceneState->contextEntityId
					: sceneState->selectedEntityId;

			if (parentId.has_value())
			{
				if (!InsertChildNode(sceneState->hierarchyNodes, parentId.value(), node))
				{
					sceneState->hierarchyNodes.push_back(std::move(node));
				}
			}
			else
			{
				sceneState->hierarchyNodes.push_back(std::move(node));
			}

			sceneState->selectedEntityId = entityId;
			sceneState->contextEntityId.reset();
			refreshHierarchyView();
			refreshInspectorView();
			logger->Log(CP_LOG_EVENT(cp::ILogger::Info, "ECS", cp::Message::Create("Created entity {}", entity.index)));
		};

		hierarchyView->SetEntityActivatedHandler([sceneState, refreshInspectorView](const cp::editorui::EntityId _entityId)
		{
			sceneState->selectedEntityId = _entityId;
			sceneState->contextEntityId.reset();
			refreshInspectorView();
		});

		hierarchyView->SetContextMenuHandler([this, sceneState, createEntityAction](const cp::editorui::EntityId _entityId)
		{
			sceneState->contextEntityId = _entityId;
			sceneState->selectedEntityId = _entityId;
			hierarchyView->SetSelectedEntity(_entityId);

			const auto menu = backend->CreateContextMenu("hierarchy.context");
			menu->AddAction(createEntityAction);
			return menu;
		});

		createEntityAction->SetTriggeredHandler([createEntity]()
		{
			createEntity();
		});

		for (ComponentAuthoringBinding& binding : *componentAuthoringBindings)
		{
			ComponentAuthoringBinding* bindingPtr = &binding;
			binding.addAction->SetTriggeredHandler([this, sceneState, selectedEntityResolver, refreshInspectorView, bindingPtr]()
			{
				const std::optional<cp::ecs::Entity> selectedEntity = selectedEntityResolver();
				if (!selectedEntity.has_value())
				{
					logger->Log(CP_LOG_EVENT(cp::ILogger::Warning, "ECS", cp::Message::Create("No selected entity to add {}", bindingPtr->authoring->Name())));
					sceneState->contextEntityId.reset();
					return;
				}

				if (bindingPtr->authoring->HasComponent(scene->GetWorld(), selectedEntity.value()))
				{
					logger->Log(CP_LOG_EVENT(cp::ILogger::Info, "ECS", cp::Message::Create("{} already exists on entity {}", bindingPtr->authoring->Name(), selectedEntity->index)));
					sceneState->contextEntityId.reset();
					refreshInspectorView();
					return;
				}

				if (bindingPtr->authoring->AddComponent(scene->GetWorld(), selectedEntity.value()))
				{
					logger->Log(CP_LOG_EVENT(cp::ILogger::Info, "ECS", cp::Message::Create("Added {} to entity {}", bindingPtr->authoring->Name(), selectedEntity->index)));
				}
				else
				{
					logger->Log(CP_LOG_EVENT(cp::ILogger::Warning, "ECS", cp::Message::Create("Failed to add {} to entity {}", bindingPtr->authoring->Name(), selectedEntity->index)));
				}

				sceneState->contextEntityId.reset();
				refreshInspectorView();
			});

			binding.removeAction->SetTriggeredHandler([this, sceneState, selectedEntityResolver, refreshInspectorView, bindingPtr]()
			{
				const std::optional<cp::ecs::Entity> selectedEntity = selectedEntityResolver();
				if (!selectedEntity.has_value())
				{
					logger->Log(CP_LOG_EVENT(cp::ILogger::Warning, "ECS", cp::Message::Create("No selected entity to remove {}", bindingPtr->authoring->Name())));
					sceneState->contextEntityId.reset();
					return;
				}

				if (!bindingPtr->authoring->HasComponent(scene->GetWorld(), selectedEntity.value()))
				{
					logger->Log(CP_LOG_EVENT(cp::ILogger::Info, "ECS", cp::Message::Create("{} does not exist on entity {}", bindingPtr->authoring->Name(), selectedEntity->index)));
					sceneState->contextEntityId.reset();
					refreshInspectorView();
					return;
				}

				if (bindingPtr->authoring->RemoveComponent(scene->GetWorld(), selectedEntity.value()))
				{
					logger->Log(CP_LOG_EVENT(cp::ILogger::Info, "ECS", cp::Message::Create("Removed {} from entity {}", bindingPtr->authoring->Name(), selectedEntity->index)));
				}
				else
				{
					logger->Log(CP_LOG_EVENT(cp::ILogger::Warning, "ECS", cp::Message::Create("Failed to remove {} from entity {}", bindingPtr->authoring->Name(), selectedEntity->index)));
				}

				sceneState->contextEntityId.reset();
				refreshInspectorView();
			});
		}

		loadSceneAction->SetTriggeredHandler([this, activeScenePath, loadScene]()
		{
			const auto fileDialog = backend->CreateFileDialog("loadSceneDialog");
			fileDialog->SetMode(cp::editorui::IFileDialog::Mode::OpenFile);
			fileDialog->SetTitle("Load Scene");
			fileDialog->SetDirectory(activeScenePath->parent_path());
			fileDialog->SetFilter("Scene Files (*.scene);;All Files (*)");

			if (fileDialog->Execute())
			{
				if (const auto selectedPath = fileDialog->GetSelectedPath())
				{
					loadScene(selectedPath.value());
					return;
				}
			}

			logger->Log(CP_LOG_EVENT(cp::ILogger::Info, "ECS", cp::Message::Create("Scene load cancelled")));
		});

		saveSceneAction->SetTriggeredHandler([this, activeScenePath, saveScene]()
		{
			const auto fileDialog = backend->CreateFileDialog("saveSceneDialog");
			fileDialog->SetMode(cp::editorui::IFileDialog::Mode::SaveFile);
			fileDialog->SetTitle("Save Scene");
			fileDialog->SetDirectory(activeScenePath->parent_path());
			fileDialog->SetDefaultFileName(activeScenePath->filename().string());
			fileDialog->SetFilter("Scene Files (*.scene);;All Files (*)");

			if (fileDialog->Execute())
			{
				if (const auto selectedPath = fileDialog->GetSelectedPath())
				{
					*activeScenePath = selectedPath.value();
					saveScene();
					return;
				}
			}

			logger->Log(CP_LOG_EVENT(cp::ILogger::Info, "ECS", cp::Message::Create("Scene save cancelled")));
		});

		createEntity();
		refreshInspectorView();

		assetView->SetAssetOpenedHandler([this, activeScenePath, loadScene](const std::filesystem::path& _path)
		{
			logger->Log(CP_LOG_EVENT(cp::ILogger::Info, "Assets", cp::Message::Create("Opening '{}'", _path.string())));

			if (_path.extension() == ".scene")
			{
				loadScene(_path);
				return;
			}

			if (assetRegistryInitialized)
			{
				auto& meshManager = cp::AssetRegistry::Instance().Get<cp::Mesh>();
				const cp::AssetHandle<cp::Mesh> mesh = meshManager.Load(_path);
				if (mesh.IsValid())
				{
					logger->Log(CP_LOG_EVENT(
						cp::ILogger::Info,
						"Resources",
						cp::Message::Create("Loaded mesh '{}' ({} vertices / {} indices)", _path.string(), mesh->GetVertices().size(), mesh->GetIndices().size())
					));
				}
				else
				{
					logger->Log(CP_LOG_EVENT(cp::ILogger::Warning, "Resources", cp::Message::Create("Unsupported asset '{}'", _path.string())));
				}
			}
		});

		const void* nativeHandle = viewportView->GetNativeRenderSurfaceHandle();
		if (nativeHandle != nullptr)
		{
			rhi = std::make_unique<cp::VulkanRHI>(logger);
			cp::InstanceInfo instanceInfo;
			instanceInfo.appName = "Checkpoint Editor";
			instanceInfo.appVersion = CP_VERSION;
			instanceInfo.enableValidationLayers = false;

			rhi->CreateInstance(instanceInfo);
			rhi->CreatePhysicalDevice();
			rhi->CreateDevice();

			cp::AssetRegistry::Instance().Initialize(*rhi);
			assetRegistryInitialized = true;

			cp::RendererInfo rendererInfo;
			rendererInfo.frameCount = 3;
			rendererInfo.extent = cp::Extent2D<int>(windowConfig.width, windowConfig.height);
			rendererInfo.imageFormat = cp::Format::R8G8B8A8_UNORM;
			rendererInfo.nativeWindowHandle = const_cast<void*>(nativeHandle);
			rendererInfo.registryManager = registryManager.get();
			rendererInfo.ecsWorld = &scene->GetWorld();

			renderer = std::make_unique<cp::Renderer>(rendererInfo, *rhi);
			cp::rendering::ApplyFrameGraphConfigToRenderer(*renderer, scene->GetActivePassNames());

			viewportView->SetResizeHandler([this](const int _width, const int _height)
			{
				if (_width <= 0 || _height <= 0 || !renderer)
				{
					return;
				}

				renderer->Resize(cp::Extent2D<int>(_width, _height));
			});
			viewportView->SetFrameHandler([this]()
			{
				{
					std::vector<cp::editorui::ConsoleEntry> pending;
					{
						std::lock_guard lock(pendingEntriesMutex);
						pending = std::move(pendingConsoleEntries);
					}

					for (auto& entry : pending)
					{
						AppendConsoleEntry(std::move(entry));
					}
				}

				// Detect when the runtime window is closed by the user
				if (runtimeApp != nullptr && !runtimeApp->IsRunning())
				{
					OnRuntimeStopped();
				}

				if (!renderer)
				{
					return;
				}

				renderer->BeginFrame();
				renderer->Render();
				renderer->EndFrame();
			});

			logger->Log(CP_LOG_EVENT(cp::ILogger::Info, "Viewport", cp::Message::Create("Renderer attached to viewport surface")));
		}
		else
		{
			logger->Log(CP_LOG_EVENT(cp::ILogger::Warning, "Viewport", cp::Message::Create("Native viewport surface is unavailable")));
		}

		dockHost->DockPanel(hierarchyPanel, { cp::editorui::DockArea::Left });
		dockHost->DockPanel(assetsPanel, { cp::editorui::DockArea::Left, "hierarchy", true });
		dockHost->DockPanel(inspectorPanel, { cp::editorui::DockArea::Right });
		dockHost->DockPanel(sceneConfigPanel, { cp::editorui::DockArea::Right, "inspector", true });
		dockHost->DockPanel(viewportPanel, { cp::editorui::DockArea::Center });
		dockHost->DockPanel(consolePanel, { cp::editorui::DockArea::Bottom });

		window->Show();
	}

	void EditorWorkspace::OnRuntimeStopped()
	{
		if (runtimeApp == nullptr)
		{
			return;
		}

		runtimeApp->Stop();
		runtimeApp.reset();

		runAction->SetEnabled(true);
		stopAction->SetEnabled(false);

		if (loadSceneDelegate && !runtimeBackupPath.empty() && std::filesystem::exists(runtimeBackupPath))
		{
			loadSceneDelegate(runtimeBackupPath);
			std::filesystem::remove(runtimeBackupPath);
		}

		runtimeBackupPath.clear();
		logger->Log(CP_LOG_EVENT(cp::ILogger::Info, "Run", cp::Message::Create("Runtime stopped, scene restored")));
	}
}
