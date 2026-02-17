#pragma once

namespace cp
{
	class ILogger;

	struct RHISurfaceInfo
	{
		void* nativeHandle = nullptr;
	};

	class RHISurface
	{
	public:
		RHISurface(RHISurfaceInfo _info, ILogger& _logger);
		virtual ~RHISurface() = default;

	protected:
		RHISurfaceInfo info;

		ILogger& logger;
	};
}