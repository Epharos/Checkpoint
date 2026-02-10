#include "RenderingHardwareInterface.hpp"

namespace cp
{
	RenderingHardwareInterface::RenderingHardwareInterface(std::shared_ptr<cp::ILogger> _logger)
		: logger(_logger)
	{

	}

	ILogger& RenderingHardwareInterface::GetLogger() const
	{
		return *logger;
	}
}