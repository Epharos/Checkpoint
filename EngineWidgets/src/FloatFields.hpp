#pragma once

#include "pch.hpp"
#include "Float.hpp"

namespace cp
{
	class Float32 : public FloatWidget<float>
	{
		Q_OBJECT

	public:
		Float32(float* _val, const std::string& _fieldName, QWidget* parent = nullptr) : FloatWidget(_val, _fieldName, parent)
		{
		}
	};

	class Float64 : public FloatWidget<double>
	{
		Q_OBJECT

	public:
		Float64(double* _val, const std::string& _fieldName, QWidget* parent = nullptr) : FloatWidget(_val, _fieldName, parent)
		{
		}
	};

	typedef Float32 Float16; // For compatibility with "half" Slang type
	typedef Float16 Half; // Alias for Float16, commonly used in graphics programming
	typedef Float32 Float;
	typedef Float64 Double;
}