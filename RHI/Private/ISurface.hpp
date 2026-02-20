#pragma once

namespace cp
{
	class ILogger;

	struct SurfaceInfo
	{
		void* nativeHandle = nullptr;
	};

	class ISurface
	{
	public:
		ISurface(SurfaceInfo _info, ILogger& _logger);
		virtual ~ISurface() = default;

	protected:
		SurfaceInfo info;

		ILogger& logger;
	};
}