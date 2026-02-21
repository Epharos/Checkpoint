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
		ISurface(ILogger& _logger, const SurfaceInfo& _info);
		virtual ~ISurface() = default;

	protected:
		SurfaceInfo info;

		ILogger& logger;
	};
}