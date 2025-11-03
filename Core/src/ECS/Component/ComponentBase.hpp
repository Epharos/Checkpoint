#pragma once

#include "pch.hpp"

namespace cp
{
	template<typename T>
	class ComponentBaseHelper
	{

	};

	class IComponentBase
	{
	protected:
		virtual ~IComponentBase() = default;
	};

	template<class T>
	concept ComponentBase = std::is_base_of<IComponentBase, T>::value;
};