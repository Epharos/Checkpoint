#include "ISurface.hpp"

namespace cp
{
	cp::ISurface::ISurface(ILogger& _logger, const SurfaceInfo& _info)
		: info(_info), logger(_logger)
	{
		
	}
}