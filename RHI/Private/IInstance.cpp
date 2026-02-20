#include "IInstance.hpp"

#include <Log.hpp>

namespace cp
{
	IInstance::IInstance(ILogger& _logger, const InstanceInfo& _instanceInfo) : logger(_logger), instanceInfo(_instanceInfo)
	{

	}
}