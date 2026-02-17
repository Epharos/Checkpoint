#include <iostream>

#include <Log.hpp>
#include <VulkanRHI.hpp>
#include <Macros.hpp>
#include <Profiling.hpp>
#include <GLFWWindow.hpp>

auto main(int argc, char** argv) -> int
{
	{
		CP_PROFILE_SCOPE("Application");
		constexpr const char* InitLabel = "Init";

		const std::shared_ptr<cp::IMessageVisitor> messageVisitor = std::make_shared<cp::ConsoleVisitor>();
		const std::shared_ptr<cp::IMessageVisitor> fileMessageVisitor = std::make_shared<cp::FileVisitor>();

		std::shared_ptr<cp::ILogger> logger = std::make_shared<cp::ConsoleLogger>(messageVisitor);
		std::shared_ptr<cp::ILogger> fileLogger = std::make_shared<cp::FileLogger>(fileMessageVisitor, "log.txt");
		std::shared_ptr<cp::CompositeLogger> compositeLogger = std::make_shared<cp::CompositeLogger>();
		compositeLogger->AddLogger(logger);
		compositeLogger->AddLogger(fileLogger);

		compositeLogger->Log(CP_LOG_EVENT(cp::ILogger::Info, InitLabel, cp::Message::Create<cp::TextComponent>("Hello, World!")));

		compositeLogger->Spacing(1);

		compositeLogger->Log(CP_LOG_EVENT(cp::ILogger::Trace, InitLabel, cp::Message::Create<cp::TextComponent>("This is a trace message.")));
		compositeLogger->Log(CP_LOG_EVENT(cp::ILogger::Debug, InitLabel, cp::Message::Create<cp::TextComponent>("This is a debug message.")));
		compositeLogger->Log(CP_LOG_EVENT(cp::ILogger::Info, InitLabel, cp::Message::Create<cp::TextComponent>("This is an info message.")));
		compositeLogger->Log(CP_LOG_EVENT(cp::ILogger::Warning, InitLabel, cp::Message::Create<cp::TextComponent>("This is a warning message.")));
		compositeLogger->Log(CP_LOG_EVENT(cp::ILogger::Error, InitLabel, cp::Message::Create<cp::TextComponent>("This is an error message.")));
		compositeLogger->Log(CP_LOG_EVENT(cp::ILogger::Critical, InitLabel, cp::Message::Create<cp::TextComponent>("This is a critical message.")));

		compositeLogger->Spacing(1);

		std::unique_ptr<cp::IWindow> window = nullptr;
		{
			CP_PROFILE_SCOPE("Window Creation");
			window = std::make_unique<cp::GLFWWindow>(cp::WindowInfo{ .title = "Test Window", .width = 800, .height = 600, .resizable = true });
		}

		{
			CP_PROFILE_SCOPE("Rendering Hardware Interface");
			std::unique_ptr<cp::RenderingHardwareInterface> rhi = std::make_unique<cp::VulkanRHI>(compositeLogger);

			cp::RHIInstanceInfo instanceInfo
			{
				.appName = "TestApp",
				.appVersion = CP_MAKE_VERSION(0, 1, 0, 0),
				.enableValidationLayers = true
			};

			std::unique_ptr<cp::RHIInstance> rhiInstance = rhi->CreateInstance(instanceInfo);
			std::unique_ptr<cp::RHIPhysicalDevice> physicalDevice = rhiInstance->CreatePhysicalDevice();
			std::unique_ptr<cp::RHIDevice> device = physicalDevice->CreateDevice();

			if(!window)
			{
				compositeLogger->Log(CP_LOG_EVENT(cp::ILogger::Critical, "Main", cp::Message::Create<cp::TextComponent>("Window was not created, therefore Surface cannot be created")));
				return -1;
			}

			{
				CP_PROFILE_SCOPE("Surface Creation");

				CP_ASSERT_MSG(window->GetNativeWindowHandle() != nullptr, "Window was created but native handle is null, cannot create surface");

				cp::RHISurfaceInfo surfaceInfo
				{
					.nativeHandle = window->GetNativeWindowHandle()
				};

				std::unique_ptr<cp::RHISurface> surface = rhiInstance->CreateSurface(surfaceInfo);
			}
		}

		{
			CP_PROFILE_SCOPE("Main Loop");

			while (!window->ShouldClose())
			{
				window->PollEvents();
			}
		}
	}

	cp::ProfilerTracker::GetInstance().SerializeEvents("profile.csv");

	return 0;
}