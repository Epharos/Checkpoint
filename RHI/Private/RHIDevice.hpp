#pragma once

namespace cp
	{
	class RHIDevice
	{
	public:
		RHIDevice() = default;
		virtual ~RHIDevice() = default;

		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;
	};
}