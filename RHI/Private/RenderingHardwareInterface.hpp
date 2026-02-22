#pragma once

#include <Log.hpp>

namespace cp
{
	class IInstance;
	class IPhysicalDevice;
	class IDevice;
	class ISurface;

	struct InstanceInfo;
	struct SurfaceInfo;

	class RenderingHardwareInterface
	{
	public:
		std::shared_ptr<cp::ILogger> logger;

		RenderingHardwareInterface(std::shared_ptr<cp::ILogger> _logger);
		virtual ~RenderingHardwareInterface() = default;

		virtual IInstance& CreateInstance(const InstanceInfo& _info) = 0;
		virtual IPhysicalDevice& CreatePhysicalDevice() = 0;
		virtual IDevice& CreateDevice() = 0;

		virtual std::unique_ptr<ISurface> CreateSurface(SurfaceInfo _info) = 0;

		virtual IInstance& GetInstance() = 0;
		virtual const IInstance& GetInstance() const = 0;
		virtual IPhysicalDevice& GetPhysicalDevice() = 0;
		virtual const IPhysicalDevice& GetPhysicalDevice() const = 0;
		virtual IDevice& GetDevice() = 0;
		virtual const IDevice& GetDevice() const = 0;

		static constexpr const char* RHI_Label = "RHI";

	protected:
		ILogger& GetLogger() const;
	};

	using RHI = RenderingHardwareInterface;
}