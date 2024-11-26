#pragma once

namespace Render
{
	class Material
	{
	public:
		Material() = default;
		virtual ~Material() = default;

		virtual void CreatePipeline() = 0;
		virtual void Cleanup() = 0;
	};
}