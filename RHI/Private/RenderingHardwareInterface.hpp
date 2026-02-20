#pragma once

#include <Log.hpp>

namespace cp
{
	class IInstance;
	struct InstanceInfo;

	class RenderingHardwareInterface
	{
	public:
		std::shared_ptr<cp::ILogger> logger;

		RenderingHardwareInterface(std::shared_ptr<cp::ILogger> _logger);
		virtual ~RenderingHardwareInterface() = default;

		virtual std::unique_ptr<IInstance> CreateInstance(const InstanceInfo& _info) = 0;

		static constexpr const char* RHI_Label = "RHI";

	protected:
		ILogger& GetLogger() const;
	};

	using RHI = RenderingHardwareInterface;
}