#pragma once

#include "../pch.hpp"

class ColoredMaterial : public Render::Material
{
public:
	ColoredMaterial() = default;
	virtual ~ColoredMaterial() = default;

	void CreatePipeline() override;
	void Cleanup() override;
};