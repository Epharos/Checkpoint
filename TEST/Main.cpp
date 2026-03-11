#include <iostream>

#include <Common/Core/Log.hpp>
#include <Common/Core/Profiling.hpp>
#include <Common/Core/Macros.hpp>

#include <VulkanRHI.hpp>

#include <RHI/Core.hpp>
#include <RHI/Rendering.hpp>
#include <RHI/Data.hpp>

#include <GLFWWindow.hpp>

auto main(int argc, char** argv) -> int
{
	{
		CP_PROFILE_SCOPE_NAMED("Application");
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
			CP_PROFILE_SCOPE_NAMED("Window Creation");
			window = std::make_unique<cp::GLFWWindow>(cp::WindowInfo{ .title = "Test Window", .extent = cp::Extent2D{ 800, 600 }, .resizable = true});
		}

		{
			CP_PROFILE_SCOPE_NAMED("Rendering Hardware Interface");
			std::unique_ptr<cp::RenderingHardwareInterface> rhi = std::make_unique<cp::VulkanRHI>(compositeLogger);

			cp::InstanceInfo instanceInfo
			{
				.appName = "TestApp",
				.appVersion = CP_MAKE_VERSION(0, 1, 0, 0),
				.enableValidationLayers = true
			};

			cp::IInstance& rhiInstance = rhi->CreateInstance(instanceInfo);
			cp::IPhysicalDevice& physicalDevice = rhi->CreatePhysicalDevice();
			cp::IDevice& device = rhi->CreateDevice();

			if(!window)
			{
				compositeLogger->Log(CP_LOG_EVENT(cp::ILogger::Critical, "Main", cp::Message::Create<cp::TextComponent>("Window was not created, therefore Surface cannot be created")));
				return -1;
			}

			{
				CP_PROFILE_SCOPE_NAMED("Surface Creation");

				CP_ASSERT_MSG(window->GetNativeWindowHandle() != nullptr, "Window was created but native handle is null, cannot create surface");

				cp::SwapchainInfo swapchainInfo
				{
					.extent = window->GetExtent(),
					.imageCount = 3,
					.format = cp::Format::B8G8R8A8_UNORM,
					.nativeWindowHandle = window->GetNativeWindowHandle()
				};

				std::unique_ptr<cp::ISwapchain> swapchain = rhi->CreateSwapchain(swapchainInfo);
			}

			{
				CP_PROFILE_SCOPE_NAMED("Texture Creation");

				cp::TextureInfo textureInfo
				{
					.type = cp::TextureType::Texture2D,
					.extent = cp::Extent3D<uint32_t>{ 512, 512, 1 },
					.mipLevels = 1,
					.arrayLayers = 1,
					.format = cp::Format::R8G8B8A8_UNORM,
					.usage = cp::TextureUsage::ColorAttachment,
					.aspect = cp::TextureAspect::Color
				};

				std::shared_ptr<cp::ITexture> texture = device.CreateTexture(textureInfo);
			}

			{


				cp::TextureInfo textureInfo
				{
					.type = cp::TextureType::Texture2D,
					.extent = cp::Extent3D<uint32_t>{ 512, 512, 1 },
					.mipLevels = 1,
					.arrayLayers = 1,
					.format = cp::Format::R8G8B8A8_UNORM,
					.usage = cp::TextureUsage::ColorAttachment,
					.aspect = cp::TextureAspect::Color
				};

				cp::TextureInfo depthTextureInfo
				{
					.type = cp::TextureType::Texture2D,
					.extent = cp::Extent3D<uint32_t>{ 512, 512, 1 },
					.mipLevels = 1,
					.arrayLayers = 1,
					.format = cp::Format::D24_UNORM_S8_UINT,
					.usage = cp::TextureUsage::DepthStencilAttachment,
					.aspect = cp::TextureAspect::DepthStencil
				};

				std::shared_ptr<cp::ITexture> texture = device.CreateTexture(textureInfo);
				std::shared_ptr<cp::ITexture> depthTexture = device.CreateTexture(depthTextureInfo);

				cp::DepthStencilAttachmentInfo depthStencilAttachmentInfo
				{
					.texture = depthTexture.get(),
					.clearValue = cp::ClearDepthStencil {
						.depth = 0.5f,
						.stencil = 0
					},
				};

				cp::ColorAttachmentInfo colorAttachment
				{
					.texture = texture.get(),
					.clearValue = cp::Color(cp::ColorRGBA8(127))
				};

				CP_PROFILE_SCOPE_NAMED("Command Allocator and Command Buffer");

				std::unique_ptr<cp::ICommandAllocator> commandAllocator = rhi->CreateCommandAllocator(device.GetQueue(cp::QueueType::Graphics, 0));

				std::unique_ptr<cp::ICommandBuffer> commandBuffer = commandAllocator->Allocate();

				cp::RenderingInfo renderingInfo
				{
					.extent = cp::Extent2D<uint32_t>{ 512, 512 },
					.layers = 1,
					.colorAttachments = { colorAttachment },
					.depthStencilAttachment = depthStencilAttachmentInfo
				};

				commandBuffer->Begin();

				commandBuffer->BeginRendering(renderingInfo);
				commandBuffer->EndRendering();

				commandBuffer->End();
			}
		}

		{
			CP_PROFILE_SCOPE_NAMED("Main Loop");

			while (!window->ShouldClose())
			{
				window->PollEvents();
			}
		}
	}

	cp::ProfilerTracker::GetInstance().SerializeEvents("profile.csv");

	return 0;
}