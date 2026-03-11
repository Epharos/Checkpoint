#include "../Rendering/ISwapchain.hpp"

namespace cp
{
	ISwapchain::ISwapchain(ILogger& _logger, const SwapchainInfo& _info)
		: logger(_logger), info(_info)
	{}
}